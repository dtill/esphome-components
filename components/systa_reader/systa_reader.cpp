#include "systa_reader.h"
#include "aqua.h"
#include "aqua_ii.h"
#include "comfort.h"
#include "compact.h"
#include "esphome/core/log.h"
#include "espresso.h"
#include "modula.h"
#include "palletti_ii.h"

namespace esphome {
namespace systa_reader {

static const char *const TAG = "systa_reader";
#ifndef SYSTA_MAX_FRAMES_PER_LOOP
#define SYSTA_MAX_FRAMES_PER_LOOP 4
#endif
#ifndef SYSTA_SYNC_FC
#define SYSTA_SYNC_FC 0xFC
#endif
#ifndef SYSTA_SYNC_DISP
#define SYSTA_SYNC_DISP 0x0F
#endif

void SystaReader::setup() {
  if ((enabled_mask_ & DEV_AQUA) && !aqua_)
    aqua_ = new AquaDecoder(*this);
  if ((enabled_mask_ & DEV_AQUA_II) && !aqua_ii_)
    aqua_ii_ = new Aqua2Decoder(*this);
  if ((enabled_mask_ & DEV_MODULA) && !modula_)
    modula_ = new ModulaDecoder(*this);
  if ((enabled_mask_ & DEV_ESPRESSO) && !espresso_)
    espresso_ = new EspressoDecoder(*this);
  if ((enabled_mask_ & DEV_PALLETTI_II) && !palletti_ii_)
    palletti_ii_ = new Palletti2Decoder(*this);
  if ((enabled_mask_ & DEV_COMPACT) && !compact_)
    compact_ = new CompactDecoder(*this);
  if ((enabled_mask_ & DEV_COMFORT) && !comfort_)
    comfort_ = new ComfortDecoder(*this);
  // future:
  // if ((enabled_mask_ & DEV_SOLAR) && !solar_) solar_ = new
  // SolarDecoder(*this);
}

void SystaReader::loop() {
  // 1) Testframes alle 10s injizieren (ohne den UART-Empfang zu blockieren)
  if (!test_data_hex_.empty()) {
    const uint32_t now = millis();
    if ((now - last_inject_ms_ >= 10000U) || (now < last_inject_ms_)) {
      inject_test_frames_();
      last_inject_ms_ = now;
    }
  }

  uint8_t frames_done = 0;
  // read/parse while data available AND we haven't exceeded our per-loop budget
  while ((this->available() || (need_total_ && cur_.size() >= need_total_)) &&
         frames_done < kMaxFramesPerLoop) {
    // 1) ensure we’re synced to a start byte
    if (rx_state_ == RxState::SEEK) {
      uint8_t b;
      bool synced = false;
      while (this->available()) {
        if (!this->read_byte(&b))
          break;
        if (b == 0xFC || b == 0x0F || b == 0xFD || b == 0x0A || b == 0x0B ||
            b == 0x0C) {
          flush_skipped_("pre-sync");
          cur_.clear();
          cur_.push_back(b);
          need_total_ = 0;
          rx_state_ = RxState::COLLECT;
          synced = true;
          break;
        }
        skipped_.push_back(b);
        if (skipped_.size() >= kSkippedFlushCap)
          flush_skipped_("cap");
      }
      if (!synced)
        break; // no sync yet → wait for more UART data
    }
    // 2) COLLECT: pull just enough to decide/complete the frame
    if (rx_state_ == RxState::COLLECT) {
      // if we don't know total yet, read a few header bytes to decide
      while (this->available() &&
             (need_total_ == 0 || cur_.size() < need_total_)) {
        uint8_t b;
        if (!this->read_byte(&b))
          break;
        cur_.push_back(b);

        if (need_total_ == 0) {
          const size_t expect = expect_total_if_known_(cur_);
          if (expect == SIZE_MAX) {
            // hard desync: log the rejected partial, then resync
            ESP_LOGV(TAG, "REJECT HEX: %s", to_hex_(cur_).c_str());
            // push the bytes back into skipped_ minus the first (sync byte
            // that didn't pan out) so a fresh sync inside them can still
            // surface in the log
            for (size_t i = 1; i < cur_.size(); i++)
              skipped_.push_back(cur_[i]);
            cur_.clear();
            rx_state_ = RxState::SEEK;
            need_total_ = 0;
            break;
          } else if (expect != 0) {
            need_total_ = expect; // we now know the total size
          }
        }
      }
      // if we still don't have a full frame, give UART time to refill
      if (need_total_ == 0 || cur_.size() < need_total_)
        break;
      // 3) we have a whole frame in cur_ → dump raw, verify & route
      const std::string hex = to_hex_(cur_);
      const bool is_cmd = (cur_[0] == 0x0A || cur_[0] == 0x0B || cur_[0] == 0x0C);
      log_frame_hex_(cur_, hex);
#if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_VERBOSE
      if (cur_.size() >= 4)
        publish_hex_all(hex);
#endif
      const uint8_t calc = checksum_twos_complement_(
          std::vector<uint8_t>(cur_.begin(), cur_.end() - 1));
      const uint8_t got = cur_.back();
      if (calc != got && this->log_invalid_) {
        ESP_LOGE(TAG, "Checksum invalid (got %02X, expected %02X)", got, calc);
      }
      // COMMAND line: decode 0x0A / 0x0B / 0x0C frames (if checksum valid)
      if (is_cmd && calc == got) {
        char type[8] = "???";
        bool found = false;
        size_t label_off = 0;
        if (cur_.size() >= 6) {
          for (size_t i = 2; i + 3 <= cur_.size() - 1; i++) {
            const uint8_t a = cur_[i], b = cur_[i + 1], c = cur_[i + 2];
            if (a >= 'A' && a <= 'Z' && b >= 'A' && b <= 'Z' &&
                c >= 'A' && c <= 'Z') {
              type[0] = (char) a;
              type[1] = (char) b;
              type[2] = (char) c;
              type[3] = '\0';
              label_off = i;
              found = true;
              break;
            }
          }
        }
        if (!found && cur_.size() >= 4) {
          // Known short-command codes (per SystaBridge sources):
          //   0x02 = GET  0x14 = MON (start monitoring)
          //   0x15 = STP (stop monitoring)  0x16 = VER (get version)
          switch (cur_[2]) {
            case 0x02: strcpy(type, "GET"); break;
            case 0x14: strcpy(type, "MON"); break;
            case 0x15: strcpy(type, "STP"); break;
            case 0x16: strcpy(type, "VER"); break;
            default:   snprintf(type, sizeof(type), "%02X", cur_[2]); break;
          }
        }

        char content[32] = "";
        if (found && type[0] == 'U' && type[1] == 'H' && type[2] == 'R') {
          // 0x0B keypad: 5 BCD-style bytes after UHR: HH MM DD MM SS
          // 0x0C keypad: 2 big-endian uint16s: minutes-since-midnight, days-since-2000
          if (cur_[0] == 0x0B && label_off + 6 < cur_.size() - 1) {
            snprintf(content, sizeof(content), "%02X:%02X %02X.%02X",
                     cur_[label_off + 3], cur_[label_off + 4],
                     cur_[label_off + 5], cur_[label_off + 6]);
          } else if (cur_[0] == 0x0C && label_off + 6 < cur_.size() - 1) {
            const uint16_t mins = (uint16_t(cur_[label_off + 3]) << 8) |
                                  cur_[label_off + 4];
            uint16_t days = (uint16_t(cur_[label_off + 5]) << 8) |
                            cur_[label_off + 6];
            int yy = 2000;
            while (true) {
              bool leap = (yy % 4 == 0 && (yy % 100 != 0 || yy % 400 == 0));
              int yd = leap ? 366 : 365;
              if ((int) days < yd) break;
              days -= yd;
              yy++;
            }
            int md[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
            if (yy % 4 == 0 && (yy % 100 != 0 || yy % 400 == 0))
              md[1] = 29;
            int mo = 0;
            while (mo < 12 && (int) days >= md[mo]) {
              days -= md[mo];
              mo++;
            }
            snprintf(content, sizeof(content), "%02u:%02u %02u.%02u.%04d",
                     unsigned(mins / 60), unsigned(mins % 60),
                     unsigned(days + 1), unsigned(mo + 1), yy);
          }
        }

        if (content[0] != '\0') {
          ESP_LOGD(TAG, "COMMAND: type: %s content: [%s] hex: [%s]", type,
                   content, hex.c_str());
        } else {
          ESP_LOGD(TAG, "COMMAND: type: %s hex: [%s]", type, hex.c_str());
        }
      }
      if (cur_[0] == 0xFC) {
        // payload for device decoders
        std::vector<uint8_t> payload(cur_.begin() + 4, cur_.end() - 1);
        // broadcast raw HEX (if you have sinks_)
        for (auto *s : sinks_)
          s->publish_frame_hex(hex);
        route_fc_frame_to_device_(cur_, payload, hex);
      } else if (cur_[0] == 0x0F) { // display
        std::vector<uint8_t> payload(cur_.begin() + 4, cur_.end() - 1);
        route_display_frame_to_device_(cur_, payload, hex);
      } else if (cur_[0] == 0xFD && cur_.size() == 8 && cur_[1] == 0x05 &&
                 cur_[2] == 0xAA) {
        // Firmware-version announce: FD 05 AA <addr> <major> <minor> <patch> <chk>
        // 0x0B = Aqua/Solar, 0x0C = Comfort (per SystaBridge).
        // Routed to the per-device decoder so the version-parsing logic
        // lives next to the rest of that device's code.
        const uint8_t addr = cur_[3];
        if (addr == 0x0B && (enabled_mask_ & DEV_AQUA) && aqua_) {
          aqua_->on_fd_version_frame(cur_[4], cur_[5], cur_[6]);
        } else if (addr == 0x0C && (enabled_mask_ & DEV_COMFORT) && comfort_) {
          comfort_->on_fd_version_frame(cur_[4], cur_[5], cur_[6]);
        }
      }
      // 4) reset for next frame (there may already be more bytes pending)
      cur_.clear();
      need_total_ = 0;
      rx_state_ = RxState::SEEK;
      frames_done++;
    }
  }
}

void SystaReader::process_buffer_() {
  // Verarbeite pro loop nur wenige Frames → schneller zurück zu UART
  size_t frames = 0;

  while (true) {
    // Mindestgröße für jeden Header
    if (buf_.size() < 4)
      return;

    // Auf Sync-Byte vorspulen (0xFC = FC-Frame, 0x0F = Display-Frame)
    while (!buf_.empty() && buf_.front() != SYSTA_SYNC_FC &&
           buf_.front() != SYSTA_SYNC_DISP) {
      buf_.pop_front();
    }
    if (buf_.size() < 4)
      return;

    bool progressed = false;
    const uint8_t lead = buf_.front();

    if (lead == SYSTA_SYNC_DISP) {
      progressed = this->try_parse_display_frame_();
    } else { // == SYSTA_SYNC_FC
      progressed = this->try_parse_fc_frame_();
    }

    if (!progressed) {
      // Desync → 1 Byte verwerfen, erneut versuchen
      buf_.pop_front();
      continue;
    }

    // Ein gültiger Frame wurde geparst
    frames++;
    if (frames >= SYSTA_MAX_FRAMES_PER_LOOP) {
// Zeit an RX/RTOS zurückgeben (bes. wichtig auf ESP8266/soft UART)
#if defined(ARDUINO_ARCH_ESP8266)
      yield(); // = delay(0)
#endif
      return;
    }
  }
}

bool SystaReader::try_parse_fc_frame_() {
  // 0) Schnell raus, wenn Header noch nicht komplett
  if (buf_.size() < 3)
    return false;

  // 1) Fixe Längenprüfung (Header=0xFC, len, ... , checksum)
  if (buf_[0] != 0xFC)
    return false;
  const uint8_t len = buf_[1];
  if (len < 2) { // plausibel machen (Payload mind. Funktionsbytes)
#if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_VERBOSE
    if (log_invalid_)
      ESP_LOGV(TAG, "Reject FC len=%u", len);
#endif
    buf_.pop_front(); // 0xFC verwerfen -> neu syncen
    return true;
  }

  const size_t total = size_t(2) + len + 1; // 2 Header + payload + checksum
  if (buf_.size() < total)
    return false; // noch nicht komplett → später nochmal

  // 2) Checksumme ohne Kopien berechnen (two's complement über alles außer
  // letztem Byte)
  uint32_t sum = 0;
  for (size_t i = 0; i < total - 1; i++)
    sum += buf_[i];
  const uint8_t calc = static_cast<uint8_t>(0 - static_cast<int>(sum & 0xFF));
  const uint8_t got = buf_[total - 1];
  const bool checksum_ok = (calc == got);

  // 3) Payload-View (ohne Kopie) – Indizes merken
  const size_t payload_begin = 4;       // nach FC,len,f2,f3
  const size_t payload_end = total - 1; // vor checksum
  const size_t payload_size =
      (payload_end > payload_begin) ? (payload_end - payload_begin) : 0;

  // 4) HEX/Text erzeugen nur wenn nötig (nach dem Konsum), aber Filter-Routing
  // braucht f2/f3 jetzt:
  const uint8_t f2 = buf_[2];
  const uint8_t f3 = buf_[3];

  // 5) Bytes JETZT konsumieren, damit der UART-Puffer schnell frei wird
  std::vector<uint8_t> frame;
  frame.reserve(total);
  std::vector<uint8_t> payload;
  payload.reserve(payload_size);

  for (size_t i = 0; i < total; i++) {
    uint8_t b = buf_.front();
    buf_.pop_front();
    frame.push_back(b);
    if (i >= payload_begin && i < payload_end)
      payload.push_back(b);
  }

  if (!checksum_ok) {
    if (log_invalid_)
      ESP_LOGE(TAG, "FC checksum invalid (got %02X, expected %02X)", got, calc);
    return true;
  }
  std::string hex;
  if (!sinks_.empty()) {
    hex = to_hex_(frame);
    for (auto *s : sinks_)
      s->publish_frame_hex(hex);
  }
  #if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_VERBOSE
        ESP_LOGV(TAG, "FC HEX f2=%02X f3=%02X len=%u", f2, f3, len);
  #endif
  this->route_fc_frame_to_device_(frame, payload, hex);

  return true;
}

bool SystaReader::try_parse_display_frame_() {
  if (buf_.size() < 4)
    return false;
  if (!(buf_[0] == 0x0F && buf_[1] == 0x22 && buf_[2] == 0x04 &&
        buf_[3] == 0x00))
    return false;

  const size_t total = 37; // 0F 22 04 00 + 32 + 1
  if (buf_.size() < total)
    return false;

  std::vector<uint8_t> frame(total);
  for (size_t i = 0; i < total; i++)
    frame[i] = buf_[i];

  const uint8_t calc = checksum_twos_complement_(
      std::vector<uint8_t>(frame.begin(), frame.end() - 1));
  const uint8_t got = frame.back();
  if (calc != got) {
    if (log_invalid_)
      ESP_LOGE(TAG, "Display checksum invalid (got %02X, expected %02X)", got,
               calc);
    // trotzdem Bytes verwerfen, um nicht zu hängen
    for (size_t i = 0; i < total; i++)
      buf_.pop_front();
    return true;
  }

  const std::string hex = to_hex_(frame);
  // ALL-Sink (HEX) beibehalten, falls gewünscht
  publish_hex_all(hex);

  // Payload 0..31 (ASCII) an Gerätemodul geben
  std::vector<uint8_t> payload(frame.begin() + 4, frame.begin() + 36);
  this->route_display_frame_to_device_(frame, payload, hex);

  for (size_t i = 0; i < total; i++)
    buf_.pop_front();
  return true;
}

void SystaReader::route_fc_frame_to_device_(const std::vector<uint8_t> &frame,
                                            const std::vector<uint8_t> &payload,
                                            const std::string &hex) {
  if (frame.size() < 4 || frame[0] != 0xFC)
    return;
  const uint8_t f2 = frame[2];
  const uint8_t f3 = frame[3];
  // AQUA: FC .. 0B 01
  if ((enabled_mask_ & DEV_AQUA) && f2 == 0x0B && f3 == 0x01 && aqua_) {
    aqua_->on_fc_frame(frame, payload, hex);
  }
  // AQUA_II: FC .. 24 01
  if ((enabled_mask_ & DEV_AQUA_II) && f2 == 0x24 && f3 == 0x01 && aqua_ii_) {
    aqua_ii_->on_fc_frame(frame, payload, hex);
  }
  // MODULA: FC .. 0C 01
  if ((enabled_mask_ & DEV_MODULA) && f2 == 0x0C && f3 == 0x01 && modula_) {
    modula_->on_fc_frame(frame, payload, hex);
  }
  // ESPRESSO: FC .. 0C 01  (shares signature with EXPRESSO, both can receive)
  if ((enabled_mask_ & DEV_ESPRESSO) && f2 == 0x0C && f3 == 0x01 && espresso_) {
    espresso_->on_fc_frame(frame, payload, hex);
  }
  // PALLETTI2: FC .. 0C 01  (shares signature with PALLETTI, both can receive)
  if ((enabled_mask_ & DEV_PALLETTI_II) && f2 == 0x0C && f3 == 0x01 &&
      palletti_ii_) {
    palletti_ii_->on_fc_frame(frame, payload, hex);
  }
  // COMPACT: FC .. 0D 01
  if ((enabled_mask_ & DEV_COMPACT) && f2 == 0x0D && f3 == 0x01 && compact_) {
    compact_->on_fc_frame(frame, payload, hex);
  }
  // COMFORT: FC .. 0C 02
  if ((enabled_mask_ & DEV_COMFORT) && f2 == 0x0C && f3 == 0x02 && comfort_) {
    comfort_->on_fc_frame(frame, payload, hex);
  }
  // (future devices: add more blocks like above)
}

// Display frames (0x0F 22 04 00 … CHK)
void SystaReader::route_display_frame_to_device_(
    const std::vector<uint8_t> &frame, const std::vector<uint8_t> &payload,
    const std::string &hex) {
  if (frame.size() < 4 || frame[0] != 0x0F || frame[1] != 0x22 ||
      frame[2] != 0x04 || frame[3] != 0x00)
    return;
  // If only AQUA should consume display frames, keep only AQUA here.
  if ((enabled_mask_ & DEV_AQUA) && aqua_) {
    aqua_->on_display_frame(frame, payload, hex);
  }
  if ((enabled_mask_ & DEV_PALLETTI_II) && palletti_ii_) {
    palletti_ii_->on_display_frame(frame, payload, hex);
  }
  // If MODULA/ESPRESSO should also see display frames, uncomment:
  // if ((enabled_mask_ & DEV_MODULA) && modula_)
  // modula_->on_display_frame(frame, payload, hex); if ((enabled_mask_ &
  // DEV_ESPRESSO) && espresso_) espresso_->on_display_frame(frame, payload,
  // hex);
}

void SystaReader::log_frame_hex_(const std::vector<uint8_t> &frame,
                                 const std::string &hex) {
  if (frame.size() < 4)
    return;
  const bool is_std_display = (frame[0] == 0x0F && frame.size() == 37 &&
                               frame[1] == 0x22 && frame[2] == 0x04 &&
                               frame[3] == 0x00);
  const bool is_short_display =
      (frame[0] == 0x0F && !is_std_display && frame.size() < 8);
  const char *label = "?";
  switch (frame[0]) {
    case 0xFC: label = "FC"; break;
    case 0xFD: label = "FD"; break;
    case 0x0F:
      label = is_std_display ? "Display"
                             : (is_short_display ? "Display Setting"
                                                 : "Display Extra");
      break;
    case 0x0A: label = "Cmd"; break;
    case 0x0B: label = "Cmd"; break;
    case 0x0C: label = "Cmd"; break;
  }
  ESP_LOGV(TAG, "%s HEX: %s", label, hex.c_str());
}

void SystaReader::flush_skipped_(const char *reason) {
  if (skipped_.empty())
    return;
  ESP_LOGV(TAG, "SKIP HEX (%s, %u bytes): %s", reason,
           (unsigned) skipped_.size(), to_hex_(skipped_).c_str());
  skipped_.clear();
}

uint8_t SystaReader::checksum_twos_complement_(const std::vector<uint8_t> &v) {
  uint32_t sum = 0;
  for (auto b : v)
    sum += b;
  return static_cast<uint8_t>(0 - static_cast<int>(sum & 0xFF));
}

std::string SystaReader::to_hex_(const std::vector<uint8_t> &buf) {
  static const char *digits = "0123456789ABCDEF";
  std::string out;
  out.reserve(buf.size() * 2);
  for (auto b : buf) {
    out.push_back(digits[(b >> 4) & 0x0F]);
    out.push_back(digits[b & 0x0F]);
  }
  return out;
}

std::vector<uint8_t> SystaReader::hex_to_bytes_(const std::string &hex) {
  std::vector<uint8_t> out;
  out.reserve(hex.size() / 2);
  auto hexval = [](char c) -> int {
    if (c >= '0' && c <= '9')
      return c - '0';
    if (c >= 'a' && c <= 'f')
      return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F')
      return 10 + (c - 'A');
    return -1;
  };
  for (size_t i = 0; i + 1 < hex.size(); i += 2) {
    int hi = hexval(hex[i]);
    int lo = hexval(hex[i + 1]);
    if (hi < 0 || lo < 0) {
      out.clear();
      return out;
    }
    out.push_back(static_cast<uint8_t>((hi << 4) | lo));
  }
  return out;
}

// --- Test-Injector: routet JEDE hinterlegte Frame korrekt weiter ---
void SystaReader::inject_test_frames_() {
  ESP_LOGW(TAG, "##############  Test-Data injected. #############)");
  for (const auto &hex_str : test_data_hex_) {
    // 1) Hex -> Bytes
    std::vector<uint8_t> frame;
    frame.reserve(hex_str.size() / 2);
    for (size_t i = 0; i + 1 < hex_str.size(); i += 2) {
      uint8_t byte =
          (uint8_t)strtoul(hex_str.substr(i, 2).c_str(), nullptr, 16);
      frame.push_back(byte);
    }
    if (frame.size() < 3)
      continue;

    // 2) HEX form + ESP_LOGV dump — fire BEFORE checksum check so test
    //    frames are always visible in the console, even when their target
    //    decoder isn't enabled in the YAML.
    const std::string hex = to_hex_(frame);
    log_frame_hex_(frame, hex);

    // 3) Checksumme prüfen (gleich wie bei echten Frames)
    const uint8_t calc = checksum_twos_complement_(
        std::vector<uint8_t>(frame.begin(), frame.end() - 1));
    const uint8_t got = frame.back();
    if (calc != got) {
      if (log_invalid_)
        ESP_LOGE(TAG, "Test frame checksum invalid (got %02X, expected %02X)",
                 got, calc);
      continue;
    }

    // 4) Testdaten auch an alle Text-Sinks pushen
    for (auto *s : sinks_)
      s->publish_frame_hex(hex);
    publish_hex_all(hex);

    // 5) Gerätespezifisch routen (wie echte Frames)
    std::vector<uint8_t> payload;
    if (frame[0] == 0xFC) {
      if (frame.size() >= 4)
        payload.assign(frame.begin() + 4, frame.end() - 1);
      route_fc_frame_to_device_(frame, payload, hex);
    } else if (frame.size() == 37 && frame[0] == 0x0F && frame[1] == 0x22 &&
               frame[2] == 0x04 && frame[3] == 0x00) {
      payload.assign(frame.begin() + 4, frame.end() - 1);
      route_display_frame_to_device_(frame, payload, hex);
    }
  }
}

size_t
SystaReader::expect_total_if_known_(const std::vector<uint8_t> &v) const {
  if (v.empty())
    return 0;

  if (v[0] == 0xFC) {
    // need at least 2 bytes to know length
    if (v.size() < 2)
      return 0;
    const uint8_t len = v[1];

    // sanity cap to avoid nonsense blocking
    static constexpr size_t kMaxLen = 64; // tune for your bus
    if (len < 2 || len > kMaxLen) {
      return SIZE_MAX; // force resync
    }
    return size_t(2) + len + 1; // FC, len, payload[len], checksum
  }

  if (v[0] == 0xFD) {
    // FD <len> <data[len]> <checksum>. SystaBridge confirms subtypes up to
    // len=0x2F (heating-curve dumps), so cap generously rather than at 32.
    if (v.size() < 2)
      return 0;
    const uint8_t len = v[1];
    static constexpr size_t kFdMaxLen = 64;
    if (len < 1 || len > kFdMaxLen) {
      return SIZE_MAX;
    }
    return size_t(2) + len + 1;
  }

  if (v[0] == 0x0A || v[0] == 0x0B || v[0] == 0x0C) {
    // Command frames from clients on the bus.
    //  0x0A = SystaBridge / SystaWeb client (cf. SystaBridge sources)
    //  0x0B = SystaInterface (Aqua/Solar keypad)
    //  0x0C = SystaComfort keypad
    // All share: <sync> <len> <data[len]> <chk>.
    // 0x0A: on this bus only the 4-byte len=0x01 form is legitimate
    //       (any longer "0A" sequence has historically been mid-stream
    //       noise). Reject anything else to keep the log clean.
    if (v.size() < 2)
      return 0;
    const uint8_t len = v[1];
    if (v[0] == 0x0A && len != 0x01)
      return SIZE_MAX;
    static constexpr size_t kCmdMaxLen = 0x80;
    if (len < 1 || len > kCmdMaxLen)
      return SIZE_MAX;
    return size_t(2) + len + 1;
  }

  if (v[0] == 0x0F) {
    // Two flavours share the 0x0F sync:
    //   * Standard 32-char display: 0F 22 04 00 <32 ASCII> <chk>  (37 bytes)
    //   * Legacy variants (e.g. 0F 1A 81 28 …): treat as generic
    //     <sync><len><data><chk> and collect the full length. Their
    //     checksum byte is read but ignored downstream.
    if (v.size() < 2)
      return 0;
    const uint8_t len = v[1];
    static constexpr size_t kDispMaxLen = 0x40;
    if (len < 1 || len > kDispMaxLen)
      return SIZE_MAX;
    return size_t(2) + len + 1;
  }

  // not a sync byte
  return SIZE_MAX;
}

} // namespace systa_reader
} // namespace esphome

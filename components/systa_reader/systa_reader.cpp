#include "systa_reader.h"
#include "aqua.h"
#include "aqua_ii.h"
#include "modula.h"
#include "espresso.h"
#include "palletti_ii.h"
#include "compact.h"
#include "esphome/core/log.h"

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
  if ((enabled_mask_ & DEV_AQUA)     && !aqua_)     aqua_     = new AquaDecoder(*this);
  if ((enabled_mask_ & DEV_AQUA_II)  && !aqua_ii_)  aqua_ii_  = new Aqua2Decoder(*this);
  if ((enabled_mask_ & DEV_MODULA)   && !modula_)   modula_   = new ModulaDecoder(*this);
  if ((enabled_mask_ & DEV_ESPRESSO) && !espresso_) espresso_ = new EspressoDecoder(*this);
  if ((enabled_mask_ & DEV_PALLETTI_II) && !palletti_ii_) palletti_ii_ = new Palletti2Decoder(*this);
  if ((enabled_mask_ & DEV_COMPACT) && !compact_) compact_ = new CompactDecoder(*this);
  // future:
  // if ((enabled_mask_ & DEV_SOLAR) && !solar_) solar_ = new SolarDecoder(*this);
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
        if (!this->read_byte(&b)) break;
        if (b == 0xFC || b == 0x0F) {
          cur_.clear();
          cur_.push_back(b);
          need_total_ = 0;
          rx_state_ = RxState::COLLECT;
          synced = true;
          break;
        }
      }
      if (!synced) break;               // no sync yet → wait for more UART data
    }
    // 2) COLLECT: pull just enough to decide/complete the frame
    if (rx_state_ == RxState::COLLECT) {
      // if we don't know total yet, read a few header bytes to decide
      while (this->available() && (need_total_ == 0 || cur_.size() < need_total_)) {
        uint8_t b;
        if (!this->read_byte(&b)) break;
        cur_.push_back(b);

        if (need_total_ == 0) {
          const size_t expect = expect_total_if_known_(cur_);
          if (expect == SIZE_MAX) {
            // hard desync: drop first byte and go back to seeking
            cur_.clear();
            rx_state_ = RxState::SEEK;
            need_total_ = 0;
            break;
          } else if (expect != 0) {
            need_total_ = expect;  // we now know the total size
          }
        }
      }
      // if we still don't have a full frame, give UART time to refill
      if (need_total_ == 0 || cur_.size() < need_total_)
        break;
      // 3) we have a whole frame in cur_ → verify & route
      const uint8_t calc = checksum_twos_complement_(
          std::vector<uint8_t>(cur_.begin(), cur_.end() - 1));
      const uint8_t got = cur_.back();
      #if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_VERBOSE
        if (calc != got && this->log_invalid_) { ESP_LOGW(TAG, "Checksum invalid (got %02X, expected %02X)", got, calc); }
      #endif
      const std::string hex = to_hex_(cur_);
      if (cur_[0] == 0xFC) {
        // payload for device decoders
        std::vector<uint8_t> payload(cur_.begin() + 4, cur_.end() - 1);
        // broadcast raw HEX (if you have sinks_)
        for (auto *s : sinks_) s->publish_frame_hex(hex);
        // route valid/invalid alike (your decoders can ignore if header not matching)
        route_fc_frame_to_device_(cur_, payload, hex);
        #if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_VERBOSE
            ESP_LOGV(TAG, "FC HEX: %s", hex.c_str());
        #endif
      } else { // 0x0F display
        // broadcast raw ALL
        publish_hex_all(hex);
        // payload is 32 bytes between 0F 22 04 00 and checksum
        std::vector<uint8_t> payload(cur_.begin() + 4, cur_.end() - 1);
        route_display_frame_to_device_(cur_, payload, hex);
        #if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_VERBOSE
            ESP_LOGV(TAG, "Display HEX: %s", hex.c_str());
        #endif
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
    if (buf_.size() < 4) return;

    // Auf Sync-Byte vorspulen (0xFC = FC-Frame, 0x0F = Display-Frame)
    while (!buf_.empty() && buf_.front() != SYSTA_SYNC_FC && buf_.front() != SYSTA_SYNC_DISP) {
      buf_.pop_front();
    }
    if (buf_.size() < 4) return;

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
  if (buf_.size() < 3) return false;

  // 1) Fixe Längenprüfung (Header=0xFC, len, ... , checksum)
  if (buf_[0] != 0xFC) return false;
  const uint8_t len   = buf_[1];
  if (len < 2) {                 // plausibel machen (Payload mind. Funktionsbytes)
    #if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_VERBOSE
    if (log_invalid_) ESP_LOGV(TAG, "Reject FC len=%u", len);
    #endif
    buf_.pop_front();            // 0xFC verwerfen -> neu syncen
    return true;
  }

  const size_t total = size_t(2) + len + 1;  // 2 Header + payload + checksum
  if (buf_.size() < total) return false;     // noch nicht komplett → später nochmal

  // 2) Checksumme ohne Kopien berechnen (two's complement über alles außer letztem Byte)
  uint32_t sum = 0;
  for (size_t i = 0; i < total - 1; i++) sum += buf_[i];
  const uint8_t calc = static_cast<uint8_t>(0 - static_cast<int>(sum & 0xFF));
  const uint8_t got  = buf_[total - 1];
  const bool checksum_ok = (calc == got);

  // 3) Payload-View (ohne Kopie) – Indizes merken
  const size_t payload_begin = 4;            // nach FC,len,f2,f3
  const size_t payload_end   = total - 1;    // vor checksum
  const size_t payload_size  = (payload_end > payload_begin) ? (payload_end - payload_begin) : 0;

  // 4) HEX/Text erzeugen nur wenn nötig (nach dem Konsum), aber Filter-Routing braucht f2/f3 jetzt:
  const uint8_t f2 = buf_[2];
  const uint8_t f3 = buf_[3];

  // 5) Bytes JETZT konsumieren, damit der UART-Puffer schnell frei wird
  std::vector<uint8_t> frame;     frame.reserve(total);
  std::vector<uint8_t> payload;   payload.reserve(payload_size);

  for (size_t i = 0; i < total; i++) {
    uint8_t b = buf_.front();
    buf_.pop_front();
    frame.push_back(b);
    if (i >= payload_begin && i < payload_end) payload.push_back(b);
  }

  if (!checksum_ok) {
    if (log_invalid_) ESP_LOGW(TAG, "FC checksum invalid (got %02X, expected %02X)", got, calc);
    return true;
  }
  std::string hex;
  if (!sinks_.empty()) {
    hex = to_hex_(frame);
    for (auto *s : sinks_) s->publish_frame_hex(hex);
  }
  #if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_VERBOSE
  ESP_LOGV(TAG, "FC HEX f2=%02X f3=%02X len=%u", f2, f3, len);
  #endif
  this->route_fc_frame_to_device_(frame, payload, hex);

  return true;
}


bool SystaReader::try_parse_display_frame_() {
  if (buf_.size() < 4) return false;
  if (!(buf_[0]==0x0F && buf_[1]==0x22 && buf_[2]==0x04 && buf_[3]==0x00)) return false;

  const size_t total = 37;  // 0F 22 04 00 + 32 + 1
  if (buf_.size() < total) return false;

  std::vector<uint8_t> frame(total);
  for (size_t i=0;i<total;i++) frame[i]=buf_[i];

  const uint8_t calc = checksum_twos_complement_(std::vector<uint8_t>(frame.begin(), frame.end()-1));
  const uint8_t got  = frame.back();
  if (calc != got) {
    if (log_invalid_) ESP_LOGW(TAG, "Display checksum invalid (got %02X, expected %02X)", got, calc);
    // trotzdem Bytes verwerfen, um nicht zu hängen
    for (size_t i=0;i<total;i++) buf_.pop_front();
    return true;
  }

  const std::string hex = to_hex_(frame);
  // ALL-Sink (HEX) beibehalten, falls gewünscht
  publish_hex_all(hex);

  // Payload 0..31 (ASCII) an Gerätemodul geben
  std::vector<uint8_t> payload(frame.begin()+4, frame.begin()+36);
  this->route_display_frame_to_device_(frame, payload, hex);

  for (size_t i=0;i<total;i++) buf_.pop_front();
  return true;
}

void SystaReader::route_fc_frame_to_device_(const std::vector<uint8_t>& frame,
                                            const std::vector<uint8_t>& payload,
                                            const std::string &hex) {
  if (frame.size() < 4 || frame[0] != 0xFC) return;
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
  if ((enabled_mask_ & DEV_PALLETTI_II) && f2 == 0x0C && f3 == 0x01 && palletti_ii_) {
    palletti_ii_->on_fc_frame(frame, payload, hex);
  }
  // COMPACT: FC .. 0D 01
  if ((enabled_mask_ & DEV_COMPACT) && f2 == 0x0D && f3 == 0x01 && compact_) {
    compact_->on_fc_frame(frame, payload, hex);
  }
  // (future devices: add more blocks like above)
}

// Display frames (0x0F 22 04 00 … CHK)
void SystaReader::route_display_frame_to_device_(const std::vector<uint8_t>& frame,
                                                 const std::vector<uint8_t>& payload,
                                                 const std::string &hex) {
  if (frame.size() < 4 || frame[0] != 0x0F || frame[1] != 0x22 || frame[2] != 0x04 || frame[3] != 0x00)
    return;
  // If only AQUA should consume display frames, keep only AQUA here.
  if ((enabled_mask_ & DEV_AQUA) && aqua_) {aqua_->on_display_frame(frame, payload, hex);}
  if ((enabled_mask_ & DEV_PALLETTI_II) && palletti_ii_) {palletti_ii_->on_display_frame(frame, payload, hex);}
  // If MODULA/ESPRESSO should also see display frames, uncomment:
  // if ((enabled_mask_ & DEV_MODULA) && modula_)   modula_->on_display_frame(frame, payload, hex);
  // if ((enabled_mask_ & DEV_ESPRESSO) && espresso_) espresso_->on_display_frame(frame, payload, hex);
}

uint8_t SystaReader::checksum_twos_complement_(const std::vector<uint8_t> &v) {
  uint32_t sum=0; for (auto b: v) sum+=b;
  return static_cast<uint8_t>(0 - static_cast<int>(sum & 0xFF));
}

std::string SystaReader::to_hex_(const std::vector<uint8_t> &buf) {
  static const char *digits = "0123456789ABCDEF";
  std::string out; out.reserve(buf.size()*2);
  for (auto b : buf) {
    out.push_back(digits[(b >> 4) & 0x0F]);
    out.push_back(digits[b & 0x0F]);
  }
  return out;
}

std::vector<uint8_t> SystaReader::hex_to_bytes_(const std::string &hex) {
  std::vector<uint8_t> out;
  out.reserve(hex.size() / 2);
  auto hexval = [](char c)->int {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
  };
  for (size_t i = 0; i + 1 < hex.size(); i += 2) {
    int hi = hexval(hex[i]);
    int lo = hexval(hex[i+1]);
    if (hi < 0 || lo < 0) { out.clear(); return out; }
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
      uint8_t byte = (uint8_t) strtoul(hex_str.substr(i, 2).c_str(), nullptr, 16);
      frame.push_back(byte);
    }
    if (frame.size() < 3) continue;

    // 2) (Optional) Checksumme prüfen (gleich wie bei echten Frames)
    const uint8_t calc = checksum_twos_complement_(std::vector<uint8_t>(frame.begin(), frame.end() - 1));
    const uint8_t got  = frame.back();
    if (calc != got) {
      if (log_invalid_) ESP_LOGW(TAG, "Test frame checksum invalid (got %02X, expected %02X)", got, calc);
      continue;
    }

    // 3) HEX-String in Standardform (zwecks Logging & Sinks)
    const std::string hex = to_hex_(frame);

    // 🔹 HIER: Testdaten auch an alle Text-Sinks pushen
    for (auto *s : sinks_) s->publish_frame_hex(hex);
    // und an die "ALL"-Sinks (falls du die nutzt)
    publish_hex_all(hex);

    // 4) Payload bilden
    std::vector<uint8_t> payload;
    if (frame[0] == 0xFC) {
      if (frame.size() >= 4) payload.assign(frame.begin() + 4, frame.end() - 1);
      // 5) Gerätespezifisch routen (wie echte Frames)
      route_fc_frame_to_device_(frame, payload, hex);
    } else if (frame.size() == 37 && frame[0] == 0x0F && frame[1] == 0x22 && frame[2] == 0x04 && frame[3] == 0x00) {
      payload.assign(frame.begin() + 4, frame.end() - 1);
      route_display_frame_to_device_(frame, payload, hex);
    }
  }
}


size_t SystaReader::expect_total_if_known_(const std::vector<uint8_t>& v) const {
  if (v.empty()) return 0;

  if (v[0] == 0xFC) {
    // need at least 2 bytes to know length
    if (v.size() < 2) return 0;
    const uint8_t len = v[1];

    // sanity cap to avoid nonsense blocking
    static constexpr size_t kMaxLen = 64; // tune for your bus
    if (len < 2 || len > kMaxLen) {
      return SIZE_MAX;  // force resync
    }
    return size_t(2) + len + 1;  // FC, len, payload[len], checksum
  }

  if (v[0] == 0x0F) {
    // need 4 bytes to decide if it's the known display frame
    if (v.size() < 4) return 0;
    if (v[1] == 0x22 && v[2] == 0x04 && v[3] == 0x00) {
      return 37; // fixed size for this display signature
    }
    // unknown 0x0F... header → desync
    return SIZE_MAX;
  }

  // not a sync byte
  return SIZE_MAX;
}

} // namespace systa_reader
} // namespace esphome

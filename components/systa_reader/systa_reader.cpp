#include "systa_reader.h"
#include "aqua.h"
#include "modula.h"
#include "espresso.h"
#include "esphome/core/log.h"

namespace esphome {
namespace systa_reader {

static const char *const TAG = "systa_reader";
#ifndef SYSTA_MAX_FRAMES_PER_LOOP
#define SYSTA_MAX_FRAMES_PER_LOOP 4   // <- ggf. auf 2/8 tweaken
#endif

#ifndef SYSTA_SYNC_FC
#define SYSTA_SYNC_FC 0xFC
#endif
#ifndef SYSTA_SYNC_DISP
#define SYSTA_SYNC_DISP 0x0F
#endif


void SystaReader::setup() {
  ensure_decoder_ready_();
}

void SystaReader::loop() {
  uint8_t b;
  while (this->available()) {
    if (!this->read_byte(&b)) break;
    buf_.push_back(b);
  }
  if (!buf_.empty()) process_buffer_();
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
  //    (Decoder bekommt gleich Views/Kopien in kleinen Vektoren)
  std::vector<uint8_t> frame;     frame.reserve(total);
  std::vector<uint8_t> payload;   payload.reserve(payload_size);

  // Minimal-Kopie: genau EINMAL aus der deque "am Stück" rausziehen:
  for (size_t i = 0; i < total; i++) {
    uint8_t b = buf_.front();
    buf_.pop_front();             // früh leeren!
    frame.push_back(b);
    if (i >= payload_begin && i < payload_end) payload.push_back(b);
  }

  if (!checksum_ok) {
    if (log_invalid_) ESP_LOGW(TAG, "FC checksum invalid (got %02X, expected %02X)", got, calc);
    return true; // wir haben konsumiert, weiter
  }
  std::string hex;
  if (!sinks_.empty()) {
    hex = to_hex_(frame);
    for (auto *s : sinks_) s->publish_frame_hex(hex);
  }
  #if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_VERBOSE
  ESP_LOGV(TAG, "FC HEX f2=%02X f3=%02X len=%u", f2, f3, len);
  #endif
  // 7) Routen (nur mit gültiger CRC)
  //    Achtung: Wir haben schon konsumiert – Decoder arbeitet auf unseren
  //    kleinen, lokalen Kopien `frame`/`payload` → UART ist wieder frei.
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
  const uint8_t f2 = frame[2], f3 = frame[3];

  if (device_type_ == "aqua") {
    if (f2 == 0x0B && f3 == 0x01) aqua_->on_fc_frame(frame, payload, hex);
    return;
  }
  if (device_type_ == "modula") {
    if (f2 == 0x0C && f3 == 0x01) modula_->on_fc_frame(frame, payload, hex);
    return;
  }
  if (device_type_ == "espresso") {
    if (f2 == 0x0C && f3 == 0x01) espresso_->on_fc_frame(frame, payload, hex);
    return;
  }
}

void SystaReader::route_display_frame_to_device_(const std::vector<uint8_t>& frame,
                                                 const std::vector<uint8_t>& payload,
                                                 const std::string &hex) {
  // Nur an das gewählte Device durchreichen
  if (device_type_ == "aqua") {
    if (aqua_ == nullptr) aqua_ = new AquaDecoder(*this);
    // AQUA: Display-Frames 0F 22 04 00
    if (frame.size() >= 37 && frame[0] == 0x0F && frame[1] == 0x22 && frame[2] == 0x04 && frame[3] == 0x00) {
      aqua_->on_display_frame(frame, payload, hex);
    }
  }
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

void SystaReader::ensure_decoder_ready_() {
  if (device_type_ == "aqua") {
    if (aqua_ == nullptr)   aqua_ = new AquaDecoder(*this);
  } else if (device_type_ == "modula") {
    if (modula_ == nullptr) modula_ = new ModulaDecoder(*this);
  } else if (device_type_ == "espresso") {
    if (espresso_ == nullptr) espresso_ = new EspressoDecoder(*this);
  }
}

} // namespace systa_reader
} // namespace esphome

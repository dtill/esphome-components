#include "systa_reader.h"
#include "aqua.h"
#include "modula.h"
#include "espresso.h"
#include "esphome/core/log.h"

namespace esphome {
namespace systa_reader {

static const char *const TAG = "systa_reader";

void SystaReader::loop() {
  uint8_t b;
  while (this->available()) {
    if (!this->read_byte(&b)) break;
    buf_.push_back(b);
  }
  if (!buf_.empty()) process_buffer_();
}

void SystaReader::process_buffer_() {
  // lazy create aqua decoder
  if (device_type_ == "aqua" && aqua_ == nullptr) aqua_ = new AquaDecoder(*this);

  while (true) {
    if (buf_.size() < 4) return;

    while (!buf_.empty() && buf_.front()!=0xFC && buf_.front()!=0x0F) buf_.pop_front();
    if (buf_.size() < 4) return;

    bool progressed = false;
    if (buf_.front()==0x0F)      progressed = try_parse_display_frame_();
    else if (buf_.front()==0xFC) progressed = try_parse_fc_frame_();

    if (!progressed) buf_.pop_front();
  }
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

bool SystaReader::try_parse_fc_frame_() {
  if (buf_.size() < 3) return false;

  const uint8_t len = buf_[1];
  const size_t total = size_t(2) + len + 1;
  if (len < 2) { if (log_invalid_) ESP_LOGV(TAG, "Reject FC len=%u", len); return false; }
  if (buf_.size() < total) return false;

  std::vector<uint8_t> frame(total);
  for (size_t i = 0; i < total; i++) frame[i] = buf_[i];

  const uint8_t calc = checksum_twos_complement_(std::vector<uint8_t>(frame.begin(), frame.end() - 1));
  const uint8_t got  = frame.back();
  const bool checksum_ok = (calc == got);

  if (!checksum_ok) {
    if (log_invalid_) ESP_LOGW(TAG, "FC checksum invalid (got %02X, expected %02X)", got, calc);
    // Frame verwerfen, aber aus dem Buffer entfernen, damit wir vorankommen
    for (size_t i = 0; i < total; i++) buf_.pop_front();
    return true;  // wir haben Bytes konsumiert
  }

  const std::string hex = to_hex_(frame);
  for (auto *s : sinks_) s->publish_frame_hex(hex);
  ESP_LOGV(TAG, "FC HEX: %s", hex.c_str());

  // Payload (falls Decoder es braucht)
  std::vector<uint8_t> payload(frame.begin() + 4, frame.end() - 1);

  // AB HIER nur noch gültige Frames routen
  this->route_fc_frame_to_device_(frame, payload, hex);

  // konsumieren
  for (size_t i = 0; i < total; i++) buf_.pop_front();
  return true;
}

void SystaReader::route_fc_frame_to_device_(const std::vector<uint8_t>& frame,
                                            const std::vector<uint8_t>& payload,
                                            const std::string &hex) {
  if (frame.size() < 4 || frame[0] != 0xFC) return;
  const uint8_t f2 = frame[2];
  const uint8_t f3 = frame[3];
  if (device_type_ == "aqua") {
    // AQUA: FC .. 0B 01
    if (f2 == 0x0B && f3 == 0x01) {
      if (aqua_ == nullptr) aqua_ = new AquaDecoder(*this);
      aqua_->on_fc_frame(frame, payload, hex);
    }
    return;
  }
  if (device_type_ == "modula") {
    // MODULA: FC .. 0C 01
    if (f2 == 0x0C && f3 == 0x01) {
      if (modula_ == nullptr) modula_ = new ModulaDecoder(*this);
      modula_->on_fc_frame(frame, payload, hex);
    }
    return;
  }
  if (device_type_ == "espresso") {
    // ESPRESSO: FC .. 0C 01
    if (f2 == 0x0C && f3 == 0x01) {
      if (espresso_ == nullptr) espresso_ = new EspressoDecoder(*this);
      espresso_->on_fc_frame(frame, payload, hex);
    }
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
  } else if (device_type_ == "modula") {
    if (modula_ == nullptr) modula_ = new ModulaDecoder(*this);
    if (frame.size() >= 37 && frame[0] == 0x0F && frame[1] == 0x22 && frame[2] == 0x04 && frame[3] == 0x00) {
      modula_->on_display_frame(frame, payload, hex);
    }
  } else if (device_type_ == "espresso") {
    if (espresso_ == nullptr) espresso_ = new EspressoDecoder(*this);
    if (frame.size() >= 37 && frame[0] == 0x0F && frame[1] == 0x22 && frame[2] == 0x04 && frame[3] == 0x00) {
      espresso_->on_display_frame(frame, payload, hex);
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

} // namespace systa_reader
} // namespace esphome

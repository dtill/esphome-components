#include "systa_reader.h"
#include "esphome/core/log.h"

namespace esphome {
namespace systa_reader {

static const char *const TAG = "systa_reader";

void SystaReader::loop() {
  uint8_t byte;
  while (this->available()) {
    if (this->read_byte(&byte)) {
      buf_.push_back(byte);
    } else {
      break;
    }
  }
  if (!buf_.empty())
    this->process_buffer_();
}

void SystaReader::process_buffer_() {
  while (true) {
    if (buf_.size() < 4) return;

    while (!buf_.empty() && buf_.front() != 0xFC && buf_.front() != 0x0F) {
      buf_.pop_front();
    }
    if (buf_.size() < 4) return;

    bool progressed = false;
    if (buf_.front() == 0x0F) {
      progressed = this->try_parse_display_frame_();
    } else if (buf_.front() == 0xFC) {
      progressed = this->try_parse_fc_frame_();
    }

    if (!progressed) {
      buf_.pop_front();
    }
  }
}

bool SystaReader::try_parse_display_frame_() {
  if (buf_.size() < 4) return false;
  if (!(buf_[0] == 0x0F && buf_[1] == 0x22 && buf_[2] == 0x04 && buf_[3] == 0x00)) return false;

  const size_t total = 37; // 0F 22 04 00 + 32 payload + 1 checksum
  if (buf_.size() < total) return false;

  std::vector<uint8_t> frame(total);
  for (size_t i = 0; i < total; i++) frame[i] = buf_[i];

  uint8_t calc = checksum_twos_complement_(std::vector<uint8_t>(frame.begin(), frame.end() - 1));
  uint8_t got  = frame.back();
  if (calc != got && this->log_invalid_) {
    ESP_LOGW(TAG, "Display frame checksum invalid (got %02X, expected %02X)", got, calc);
  }

  const std::string hex = to_hex_(frame);
  for (auto *s : sinks_) s->publish_frame_hex(hex);
  ESP_LOGV(TAG, "Display frame HEX: %s", hex.c_str());

  for (size_t i = 0; i < total; i++) buf_.pop_front();
  return true;
}

bool SystaReader::try_parse_fc_frame_() {
  if (buf_.size() < 3) return false;

  const uint8_t len = buf_[1];
  const size_t total = static_cast<size_t>(2) + len + 1; // 0xFC + len + payload + checksum
  if (len < 2) {
    if (this->log_invalid_) ESP_LOGV(TAG, "Rejecting FC frame with too small len=%u", len);
    return false;
  }
  if (buf_.size() < total) return false;

  std::vector<uint8_t> frame(total);
  for (size_t i = 0; i < total; i++) frame[i] = buf_[i];

  uint8_t calc = checksum_twos_complement_(std::vector<uint8_t>(frame.begin(), frame.end() - 1));
  uint8_t got  = frame.back();
  if (calc != got && this->log_invalid_) {
    ESP_LOGW(TAG, "FC frame checksum invalid (got %02X, expected %02X)", got, calc);
  }

  const std::string hex = to_hex_(frame);
  for (auto *s : sinks_) s->publish_frame_hex(hex);
  ESP_LOGV(TAG, "FC frame HEX: %s", hex.c_str());

  for (size_t i = 0; i < total; i++) buf_.pop_front();
  return true;
}

uint8_t SystaReader::checksum_twos_complement_(const std::vector<uint8_t> &data_wo) {
  uint32_t sum = 0;
  for (auto b : data_wo) sum += b;
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

}  // namespace systa_reader
}  // namespace esphome

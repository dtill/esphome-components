#include "systa_reader.h"


bool SystaReader::try_parse_fc_frame_() {
if (buf_.size() < 3) return false;
// Layout (observed): 0:0xFC, 1:len, 2:func/hi, 3:func/lo, 4..(1+len) payload, last: checksum
const uint8_t len = buf_[1];
const size_t total = static_cast<size_t>(2) + len + 1; // 2 header bytes + payload + checksum
if (len < 2) {
// minimal payload should at least contain two header bytes (func, sub)
if (this->log_invalid_)
ESP_LOGV(TAG, "Rejecting FC frame with too small len=%u", len);
return false;
}
if (buf_.size() < total) return false; // wait for full frame


std::vector<uint8_t> frame(total);
for (size_t i = 0; i < total; i++) frame[i] = buf_[i];


// Verify checksum
uint8_t calc = checksum_twos_complement_(std::vector<uint8_t>(frame.begin(), frame.end() - 1));
uint8_t got = frame.back();
if (calc != got) {
if (this->log_invalid_)
ESP_LOGW(TAG, "FC frame checksum invalid (got %02X, expected %02X)", got, calc);
}


// Emit HEX string to sinks
const std::string hex = to_hex_(frame);
for (auto *s : sinks_) s->publish_frame_hex(hex);
ESP_LOGV(TAG, "FC frame HEX: %s", hex.c_str());


// consume
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


} // namespace systa_reader
} // namespace esphome
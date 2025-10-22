#include "systa_reader.h"
#include "esphome/core/log.h"
#include "devices/aqua.h"

namespace esphome {
namespace systa_reader {

static const char *const TAG = "systa_reader";

namespace {
std::unique_ptr<DeviceBase> make_device(SystaReader &r, const std::string &type) {
  if (type == "aqua") return std::unique_ptr<DeviceBase>(new AquaDevice(r));
  // TODO: modula/espresso/solar
  return nullptr;
}
} // namespace

// --- Public API ---

void SystaReader::set_log_invalid(bool v) { log_invalid_ = v; }
void SystaReader::set_device_type(const std::string &t) { device_type_ = t; }
void SystaReader::add_sink_all(HexSink *s)  { sinks_all_.push_back(s); }
void SystaReader::add_sink_aqua(HexSink *s) { sinks_aqua_.push_back(s); }
void SystaReader::set_numeric_sensor(Kind k, sensor::Sensor *s) { num_sensors_[k] = s; }
void SystaReader::set_text_sensor(Kind k, text_sensor::TextSensor *t) { txt_sensors_[k] = t; }

void SystaReader::publish_numeric(Kind k, float v) {
  auto it = num_sensors_.find(k);
  if (it != num_sensors_.end() && it->second) it->second->publish_state(v);
}
void SystaReader::publish_text(Kind k, const std::string &v) {
  auto it = txt_sensors_.find(k);
  if (it != txt_sensors_.end() && it->second) it->second->publish_state(v);
}

void SystaReader::setup() {
  // nothing
}

void SystaReader::loop() {
  uint8_t b;
  while (this->available()) {
    if (!this->read_byte(&b)) break;
    buf_.push_back(b);
  }
  if (!buf_.empty()) process_buffer_();
}

// --- Parser intern ---

void SystaReader::process_buffer_() {
  if (!device_) device_ = make_device(*this, device_type_);

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

  const size_t total = 37;
  if (buf_.size() < total) return false;

  std::vector<uint8_t> frame(total);
  for (size_t i=0;i<total;i++) frame[i]=buf_[i];

  const uint8_t calc = checksum_twos_complement_(std::vector<uint8_t>(frame.begin(), frame.end()-1));
  const uint8_t got  = frame.back();
  if (calc != got && log_invalid_) ESP_LOGW(TAG, "Display checksum invalid (got %02X, expected %02X)", got, calc);

  const std::string hex = to_hex_(frame);
  for (auto *s : sinks_all_) s->publish_frame_hex(hex);
  ESP_LOGV(TAG, "Display HEX: %s", hex.c_str());

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
  for (size_t i=0;i<total;i++) frame[i]=buf_[i];

  const uint8_t calc = checksum_twos_complement_(std::vector<uint8_t>(frame.begin(), frame.end()-1));
  const uint8_t got  = frame.back();
  if (calc != got && log_invalid_) ESP_LOGW(TAG, "FC checksum invalid (got %02X, expected %02X)", got, calc);

  const std::string hex = to_hex_(frame);
  for (auto *s : sinks_all_) s->publish_frame_hex(hex);
  ESP_LOGV(TAG, "FC HEX: %s", hex.c_str());

  if (device_) {
    // payload ist 0-basiert ab [2] bis vor checksum; Funktionscode sitzt bei [2]/[3]
    std::vector<uint8_t> payload(frame.begin()+4, frame.end()-1);
    device_->on_fc_frame(frame, payload, hex);
  }

  for (size_t i=0;i<total;i++) buf_.pop_front();
  return true;
}

// --- statische Helfer ---

uint8_t SystaReader::checksum_twos_complement_(const std::vector<uint8_t> &v) {
  uint32_t sum=0; for (auto b: v) sum+=b;
  return static_cast<uint8_t>(0 - static_cast<int>(sum & 0xFF));
}

std::string SystaReader::to_hex_(const std::vector<uint8_t> &buf) {
  static const char *digits="0123456789ABCDEF";
  std::string out; out.reserve(buf.size()*2);
  for (auto b: buf) { out.push_back(digits[(b>>4)&0xF]); out.push_back(digits[b&0xF]); }
  return out;
}

}  // namespace systa_reader
}  // namespace esphome

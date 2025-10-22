#pragma once
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include <deque>
#include <vector>
#include <string>

namespace esphome {
namespace systa_reader {

class SystaReader : public uart::UARTDevice, public Component {
 public:
  void set_log_invalid(bool v) { this->log_invalid_ = v; }
  void set_device_type(const std::string &t) { this->device_type_ = t; }

  // AQUA sensor setters
  void set_aqua_tsa_sensor(sensor::Sensor *s) { aqua_tsa_ = s; }
  void set_aqua_tse_sensor(sensor::Sensor *s) { aqua_tse_ = s; }
  void set_aqua_twu_sensor(sensor::Sensor *s) { aqua_twu_ = s; }
  void set_aqua_tw2_sensor(sensor::Sensor *s) { aqua_tw2_ = s; }
  void set_aqua_sol_sensor(sensor::Sensor *s) { aqua_sol_ = s; }
  void set_aqua_tag_sensor(sensor::Sensor *s) { aqua_tag_ = s; }
  void set_aqua_ges_sensor(sensor::Sensor *s) { aqua_ges_ = s; }
  void set_aqua_status_code_sensor(sensor::Sensor *s) { aqua_status_code_ = s; }
  void set_aqua_status_text_sensor(text_sensor::TextSensor *t) { aqua_status_text_ = t; }
  void set_aqua_timestamp_text_sensor(text_sensor::TextSensor *t) { aqua_timestamp_ = t; }

  void setup() override {}
  void loop() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  // raw HEX sinks (vom ersten Schritt)
  class HexSink { public: virtual void publish_frame_hex(const std::string &hex) = 0; };
  void add_sink(HexSink *sink) { sinks_.push_back(sink); }

 protected:
  void process_buffer_();
  bool try_parse_fc_frame_();
  bool try_parse_display_frame_();
  void handle_aqua_payload_(const std::vector<uint8_t> &bytes, const std::vector<uint8_t> &payload);

  static uint8_t checksum_twos_complement_(const std::vector<uint8_t> &data_without_checksum);
  static std::string to_hex_(const std::vector<uint8_t> &buf);
  static uint8_t bcd2dec_(uint8_t v) { return uint8_t(((v >> 4) * 10) + (v & 0x0F)); }
  static uint16_t read_u16_be_(const std::vector<uint8_t> &b, int i) { return uint16_t((b[i] << 8) | b[i+1]); }
  static uint32_t read_u32_be_(const std::vector<uint8_t> &b, int i) { return (uint32_t(b[i])<<24) | (uint32_t(b[i+1])<<16) | (uint32_t(b[i+2])<<8) | uint32_t(b[i+3]); }

  std::deque<uint8_t> buf_{};
  std::vector<HexSink *> sinks_{};
  bool log_invalid_{true};
  std::string device_type_{"aqua"};

  // AQUA sensors
  sensor::Sensor *aqua_tsa_{nullptr};
  sensor::Sensor *aqua_tse_{nullptr};
  sensor::Sensor *aqua_twu_{nullptr};
  sensor::Sensor *aqua_tw2_{nullptr};
  sensor::Sensor *aqua_sol_{nullptr};
  sensor::Sensor *aqua_tag_{nullptr};
  sensor::Sensor *aqua_ges_{nullptr};
  sensor::Sensor *aqua_status_code_{nullptr};
  text_sensor::TextSensor *aqua_status_text_{nullptr};
  text_sensor::TextSensor *aqua_timestamp_{nullptr};
};

// concrete text sensor sink (raw hex)
class SystaReaderTextSensor : public text_sensor::TextSensor, public Component, public SystaReader::HexSink {
 public:
  void publish_frame_hex(const std::string &hex) override { this->publish_state(hex); }
};

}  // namespace systa_reader
}  // namespace esphome

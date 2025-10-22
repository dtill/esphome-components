#pragma once
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include <deque>
#include <map>
#include <vector>

namespace esphome {
namespace systa_reader {

// Messwert-Keys (stabil über YAML)
enum class Kind : uint8_t {
  TSA, TSE, TWU, TW2, SOL, TAG, GESAMT, STATUS_CODE, STATUS_TEXT, TIMESTAMP
};

class DeviceBase;

class SystaReader : public uart::UARTDevice, public Component {
 public:
  // config
  void set_log_invalid(bool v) { log_invalid_ = v; }
  void set_device_type(const std::string &t) { device_type_ = t; }

  // Sinks für RAW-HEX
  class HexSink { public: virtual void publish_frame_hex(const std::string &hex) = 0; };
  void add_sink_all(HexSink *s)  { sinks_all_.push_back(s); }
  void add_sink_aqua(HexSink *s) { sinks_aqua_.push_back(s); }

  // Registrierung: numerische Sensoren & Text-Sensoren (Felder)
  void set_numeric_sensor(Kind k, sensor::Sensor *s) { num_sensors_[k] = s; }
  void set_text_sensor(Kind k, text_sensor::TextSensor *t) { txt_sensors_[k] = t; }

  // Publish-Helfer (für Devices)
  void publish_numeric(Kind k, float v) { if (auto it=num_sensors_.find(k); it!=num_sensors_.end() && it->second) it->second->publish_state(v); }
  void publish_text(Kind k, const std::string &v) { if (auto it=txt_sensors_.find(k); it!=txt_sensors_.end() && it->second) it->second->publish_state(v); }

  // lifecycle
  void setup() override {}
  void loop() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  void process_buffer_();
  bool try_parse_fc_frame_();
  bool try_parse_display_frame_();

  static uint8_t  checksum_twos_complement_(const std::vector<uint8_t> &data_wo);
  static std::string to_hex_(const std::vector<uint8_t> &buf);

  // state
  std::deque<uint8_t> buf_{};
  std::vector<HexSink*> sinks_all_{};
  std::vector<HexSink*> sinks_aqua_{};
  bool log_invalid_{true};
  std::string device_type_{"aqua"};

  std::unique_ptr<DeviceBase> device_;

  std::map<Kind, sensor::Sensor*>        num_sensors_{};
  std::map<Kind, text_sensor::TextSensor*> txt_sensors_{};

  friend class DeviceBase;
};

// Basis-Interface für Geräte
class DeviceBase {
 public:
  explicit DeviceBase(SystaReader &owner) : r_(owner) {}
  virtual ~DeviceBase() = default;
  virtual void on_fc_frame(const std::vector<uint8_t>& frame,
                           const std::vector<uint8_t>& payload,
                           const std::string &hex) = 0;
 protected:
  static uint8_t  bcd2dec(uint8_t v) { return uint8_t(((v>>4)*10) + (v & 0x0F)); }
  static uint16_t read_u16_be(const std::vector<uint8_t> &b, int i) { return uint16_t((b[i]<<8) | b[i+1]); }
  static uint32_t read_u32_be(const std::vector<uint8_t> &b, int i) { return (uint32_t(b[i])<<24)|(uint32_t(b[i+1])<<16)|(uint32_t(b[i+2])<<8)|uint32_t(b[i+3]); }
  SystaReader &r_;
};

// Ein Textsensor, der HEX empfängt (RAW-Streams)
class SystaReaderTextSensor : public text_sensor::TextSensor, public Component, public SystaReader::HexSink {
 public:
  void publish_frame_hex(const std::string &hex) override { this->publish_state(hex); }
};

}  // namespace systa_reader
}  // namespace esphome

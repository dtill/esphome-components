#pragma once
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include <deque>
#include <map>
#include <vector>
#include <memory>
#include <string>

namespace esphome {
namespace systa_reader {

// stabile Schlüssel je Device (mit Präfix)
enum class Kind : uint16_t {
  // AQUA
  AQUA_TSA, AQUA_TSE, AQUA_TWU, AQUA_TW2, AQUA_SOL, AQUA_TAG, AQUA_GESAMT,
  AQUA_STATUS_CODE, AQUA_STATUS_TEXT, AQUA_TIMESTAMP,
  // weitere Devices hier hinzufügen...
};

class DeviceBase;

class SystaReader : public uart::UARTDevice, public Component {
 public:
  // Konfiguration
  void set_log_invalid(bool v);
  void set_device_type(const std::string &t) { device_type_ = t; }

  // RAW-HEX Sinks
  class HexSink { public: virtual void publish_frame_hex(const std::string &hex) = 0; virtual ~HexSink() = default; };
  void add_sink_all(HexSink *s);
  void add_sink_aqua(HexSink *s);

  // Registrierung von Entities (numerisch/Text)
  void set_numeric_sensor(Kind k, sensor::Sensor *s);
  void set_text_sensor(Kind k, text_sensor::TextSensor *t);

  // Publish (von Devices aufgerufen)
  void publish_numeric(Kind k, float v);
  void publish_text(Kind k, const std::string &v);

  // Component
  void setup() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  // Parser
  void process_buffer_();
  bool try_parse_fc_frame_();
  bool try_parse_display_frame_();

  // Helfer
  static uint8_t      checksum_twos_complement_(const std::vector<uint8_t> &data_wo);
  static std::string  to_hex_(const std::vector<uint8_t> &buf);

  // Zustand
  std::deque<uint8_t>                   buf_;
  std::vector<HexSink*>                 sinks_all_;
  std::vector<HexSink*>                 sinks_aqua_;
  bool                                  log_invalid_{true};
  std::string                           device_type_{"aqua"};

  std::unique_ptr<DeviceBase>           device_;
  std::map<Kind, sensor::Sensor*>       num_sensors_;
  std::map<Kind, text_sensor::TextSensor*> txt_sensors_;
};

// Basisklasse für Gerätespezifika
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

// Textsensor, der HEX-Sinks bedienen kann
class SystaReaderTextSensor : public text_sensor::TextSensor, public Component, public SystaReader::HexSink {
 public:
  void publish_frame_hex(const std::string &hex) override { this->publish_state(hex); }
};

}  // namespace systa_reader
}  // namespace esphome

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

class AquaDecoder;
class ModulaDecoder;
class SystaReaderTextSink;  // forward

class SystaReader : public uart::UARTDevice, public Component {
 public:

  // config
  void set_log_invalid(bool v) { log_invalid_ = v; }
  void set_device_type(const std::string &t) { device_type_ = t; }

  // lifecycle
  void setup() override {}
  void loop() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  // raw sinks
  class HexSink { public: virtual void publish_frame_hex(const std::string &hex) = 0; virtual ~HexSink() = default; };
  void add_sink_all(HexSink *s)  { sinks_all_.push_back(s); }
  void add_sink_aqua(HexSink *s) { sinks_aqua_.push_back(s); }

  friend class ModulaDecoder;
  friend class AquaDecoder;

  // setters (werden von Subplatforms aufgerufen)

  // AQUA numeric
  void set_aqua_tsa_sensor(sensor::Sensor *s)         { aqua_tsa_ = s; }
  void set_aqua_tse_sensor(sensor::Sensor *s)         { aqua_tse_ = s; }
  void set_aqua_twu_sensor(sensor::Sensor *s)         { aqua_twu_ = s; }
  void set_aqua_tw2_sensor(sensor::Sensor *s)         { aqua_tw2_ = s; }
  void set_aqua_sol_sensor(sensor::Sensor *s)         { aqua_sol_ = s; }
  void set_aqua_tag_sensor(sensor::Sensor *s)         { aqua_tag_ = s; }
  void set_aqua_ges_sensor(sensor::Sensor *s)         { aqua_ges_ = s; }
  void set_aqua_status_code_sensor(sensor::Sensor *s) { aqua_status_code_ = s; }
  // AQUA text
  void set_aqua_status_text_sensor(text_sensor::TextSensor *t) { aqua_status_text_ = t; }
  void set_aqua_timestamp_text_sensor(text_sensor::TextSensor *t) { aqua_timestamp_ = t; }

  // AQUA Display
  void set_aqua_display_text_sensor(text_sensor::TextSensor *t) { aqua_display_text_ = t; }
  inline void pub_aqua_display_text(const std::string &s) { if (aqua_display_text_) aqua_display_text_->publish_state(s); }


  // publish helpers
  void pub_aqua_tsa(float v)           { if (aqua_tsa_) aqua_tsa_->publish_state(v); }
  void pub_aqua_tse(float v)           { if (aqua_tse_) aqua_tse_->publish_state(v); }
  void pub_aqua_twu(float v)           { if (aqua_twu_) aqua_twu_->publish_state(v); }
  void pub_aqua_tw2(float v)           { if (aqua_tw2_) aqua_tw2_->publish_state(v); }
  void pub_aqua_sol(float v)           { if (aqua_sol_) aqua_sol_->publish_state(v); }
  void pub_aqua_tag(float v)           { if (aqua_tag_) aqua_tag_->publish_state(v); }
  void pub_aqua_ges(float v)           { if (aqua_ges_) aqua_ges_->publish_state(v); }
  void pub_aqua_status_code(float v)   { if (aqua_status_code_) aqua_status_code_->publish_state(v); }
  void pub_aqua_status_text(const std::string &s) { if (aqua_status_text_) aqua_status_text_->publish_state(s); }
  void pub_aqua_timestamp(const std::string &s)   { if (aqua_timestamp_)   aqua_timestamp_->publish_state(s); }

  // MODULA setters
  void set_modula_two_sensor(sensor::Sensor *s) { modula_two_ = s; }
  void set_modula_tbv_sensor(sensor::Sensor *s) { modula_tbv_ = s; }
  void set_modula_tbr_sensor(sensor::Sensor *s) { modula_tbr_ = s; }
  void set_modula_tv_sensor(sensor::Sensor *s)  { modula_tv_ = s; }
  void set_modula_tv2_sensor(sensor::Sensor *s) { modula_tv2_ = s; }
  void set_modula_tr_sensor(sensor::Sensor *s)  { modula_tr_ = s; }
  void set_modula_tr2_sensor(sensor::Sensor *s) { modula_tr2_ = s; }
  void set_modula_tpo_sensor(sensor::Sensor *s) { modula_tpo_ = s; }
  void set_modula_tpu_sensor(sensor::Sensor *s) { modula_tpu_ = s; }
  void set_modula_tzr_sensor(sensor::Sensor *s) { modula_tzr_ = s; }
  void set_modula_timestamp_text_sensor(text_sensor::TextSensor *t) { modula_timestamp_ = t; }

  // MODULA publish helpers
  inline void pub_modula_two(float v) { if (modula_two_) modula_two_->publish_state(v); }
  inline void pub_modula_tbv(float v) { if (modula_tbv_) modula_tbv_->publish_state(v); }
  inline void pub_modula_tbr(float v) { if (modula_tbr_) modula_tbr_->publish_state(v); }
  inline void pub_modula_tv(float v)  { if (modula_tv_)  modula_tv_->publish_state(v); }
  inline void pub_modula_tv2(float v) { if (modula_tv2_) modula_tv2_->publish_state(v); }
  inline void pub_modula_tr(float v)  { if (modula_tr_)  modula_tr_->publish_state(v); }
  inline void pub_modula_tr2(float v) { if (modula_tr2_) modula_tr2_->publish_state(v); }
  inline void pub_modula_tpo(float v) { if (modula_tpo_) modula_tpo_->publish_state(v); }
  inline void pub_modula_tpu(float v) { if (modula_tpu_) modula_tpu_->publish_state(v); }
  inline void pub_modula_tzr(float v) { if (modula_tzr_) modula_tzr_->publish_state(v); }
  inline void pub_modula_timestamp(const std::string &s) { if (modula_timestamp_) modula_timestamp_->publish_state(s); }


  // General setters
  bool log_invalid() const { return log_invalid_; }
  void publish_hex_all(const std::string &hex)  { for (auto *s : sinks_all_)  s->publish_frame_hex(hex); }
  void publish_hex_aqua(const std::string &hex) { for (auto *s : sinks_aqua_) s->publish_frame_hex(hex); }

 private:
  // parsing
  void process_buffer_();
  bool try_parse_display_frame_();
  bool try_parse_fc_frame_();

  // kleiner Router für FC-Frames (ruft nur das gewählte Gerät auf)
  void route_fc_frame_to_device_(const std::vector<uint8_t>& frame,
                                 const std::vector<uint8_t>& payload,
                                 const std::string &hex);
  // kleiner Router für 0F-Frames (ruft nur das gewählte Gerät auf)
  void route_display_frame_to_device_(const std::vector<uint8_t>& frame,
                                      const std::vector<uint8_t>& payload,
                                      const std::string &hex);

  // utils
  static uint8_t     checksum_twos_complement_(const std::vector<uint8_t> &data_wo_last);
  static std::string to_hex_(const std::vector<uint8_t> &buf);

  // state
  std::deque<uint8_t> buf_;
  std::vector<HexSink*> sinks_all_;
  std::vector<HexSink*> sinks_aqua_;
  std::vector<SystaReaderTextSink *> sinks_{};
  bool log_invalid_{true};
  std::string device_type_{"aqua"};

  // decoder instances
  AquaDecoder *aqua_{nullptr};

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
  text_sensor::TextSensor *aqua_display_text_{nullptr};

  // decoder instances
  ModulaDecoder *modula_{nullptr};

  // MODULA sensor pointers
  sensor::Sensor *modula_two_{nullptr};
  sensor::Sensor *modula_tbv_{nullptr};
  sensor::Sensor *modula_tbr_{nullptr};
  sensor::Sensor *modula_tv_{nullptr};
  sensor::Sensor *modula_tv2_{nullptr};
  sensor::Sensor *modula_tr_{nullptr};
  sensor::Sensor *modula_tr2_{nullptr};
  sensor::Sensor *modula_tpo_{nullptr};
  sensor::Sensor *modula_tpu_{nullptr};
  sensor::Sensor *modula_tzr_{nullptr};
  text_sensor::TextSensor *modula_timestamp_{nullptr};
};

// Sink-Interface (bestehend)
class SystaReaderTextSink {
 public:
  virtual void publish_frame_hex(const std::string &hex) = 0;
  virtual ~SystaReaderTextSink() = default;
};

// concrete text sensor sink
class SystaReaderTextSensor : public text_sensor::TextSensor, public Component, public SystaReader::HexSink {
 public:
  void publish_frame_hex(const std::string &hex) override { this->publish_state(hex); }
};

} // namespace systa_reader
} // namespace esphome

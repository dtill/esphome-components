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
class EspressoDecoder;
class SystaReaderTextSink;  // forward

class SystaReader : public uart::UARTDevice, public Component {
 public:
  // 32-bit space for future devices (up to 32 flags)
  static constexpr uint32_t DEV_AQUA      = (1u << 0);
  static constexpr uint32_t DEV_MODULA    = (1u << 1);
  static constexpr uint32_t DEV_ESPRESSO  = (1u << 2);
  static constexpr uint32_t DEV_SOLAR     = (1u << 3);
  // reserve more bits for future devices:
  // static constexpr uint32_t DEV_FOO   = (1u << 4);
  // static constexpr uint32_t DEV_BAR   = (1u << 5);
  // ...


  // config
  void set_log_invalid(bool v) { log_invalid_ = v; }
  void set_device_type(const std::string &t) { device_type_ = t;}

  // lifecycle
  void setup() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void set_enabled_mask(uint8_t m) { enabled_mask_ = m; }

  // raw sinks
  class HexSink { public: virtual void publish_frame_hex(const std::string &hex) = 0; virtual ~HexSink() = default; };
  void add_sink_all(HexSink *s)  { sinks_all_.push_back(s); }
  void add_sink_aqua(HexSink *s) { sinks_aqua_.push_back(s); }

  friend class ModulaDecoder;
  friend class AquaDecoder;
  friend class EspressoDecoder;

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
  inline void pub_aqua_tsa(float v)           { if (aqua_tsa_) aqua_tsa_->publish_state(v); }
  inline void pub_aqua_tse(float v)           { if (aqua_tse_) aqua_tse_->publish_state(v); }
  inline void pub_aqua_twu(float v)           { if (aqua_twu_) aqua_twu_->publish_state(v); }
  inline void pub_aqua_tw2(float v)           { if (aqua_tw2_) aqua_tw2_->publish_state(v); }
  inline void pub_aqua_sol(float v)           { if (aqua_sol_) aqua_sol_->publish_state(v); }
  inline void pub_aqua_tag(float v)           { if (aqua_tag_) aqua_tag_->publish_state(v); }
  inline void pub_aqua_ges(float v)           { if (aqua_ges_) aqua_ges_->publish_state(v); }
  inline void pub_aqua_status_code(float v)   { if (aqua_status_code_) aqua_status_code_->publish_state(v); }
  inline void pub_aqua_status_text(const std::string &s) { if (aqua_status_text_) aqua_status_text_->publish_state(s); }
  inline void pub_aqua_timestamp(const std::string &s)   { if (aqua_timestamp_)   aqua_timestamp_->publish_state(s); }

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

  // ESPRESSO Setter
  void set_espresso_t01_sensor(sensor::Sensor *s) { espresso_t01_ = s; }
  void set_espresso_t02_sensor(sensor::Sensor *s) { espresso_t02_ = s; }
  void set_espresso_t03_sensor(sensor::Sensor *s) { espresso_t03_ = s; }
  void set_espresso_t04_sensor(sensor::Sensor *s) { espresso_t04_ = s; }
  void set_espresso_t05_sensor(sensor::Sensor *s) { espresso_t05_ = s; }
  void set_espresso_pk_sensor (sensor::Sensor *s) { espresso_pk_  = s; }
  void set_espresso_t06_sensor(sensor::Sensor *s) { espresso_t06_ = s; }
  void set_espresso_t07_sensor(sensor::Sensor *s) { espresso_t07_ = s; }
  void set_espresso_t08_sensor(sensor::Sensor *s) { espresso_t08_ = s; }
  void set_espresso_t09_sensor(sensor::Sensor *s) { espresso_t09_ = s; }
  void set_espresso_t10_sensor(sensor::Sensor *s) { espresso_t10_ = s; }
  void set_espresso_t11_sensor(sensor::Sensor *s) { espresso_t11_ = s; }
  void set_espresso_t12_sensor(sensor::Sensor *s) { espresso_t12_ = s; }
  void set_espresso_timestamp_text_sensor(text_sensor::TextSensor *t) { espresso_timestamp_ = t; }
  // ESPRESSO: private Publisher
  inline void pub_espresso_t01(float v){ if (espresso_t01_) espresso_t01_->publish_state(v); }
  inline void pub_espresso_t02(float v){ if (espresso_t02_) espresso_t02_->publish_state(v); }
  inline void pub_espresso_t03(float v){ if (espresso_t03_) espresso_t03_->publish_state(v); }
  inline void pub_espresso_t04(float v){ if (espresso_t04_) espresso_t04_->publish_state(v); }
  inline void pub_espresso_t05(float v){ if (espresso_t05_) espresso_t05_->publish_state(v); }
  inline void pub_espresso_pk (float v){ if (espresso_pk_)  espresso_pk_->publish_state(v); }
  inline void pub_espresso_t06(float v){ if (espresso_t06_) espresso_t06_->publish_state(v); }
  inline void pub_espresso_t07(float v){ if (espresso_t07_) espresso_t07_->publish_state(v); }
  inline void pub_espresso_t08(float v){ if (espresso_t08_) espresso_t08_->publish_state(v); }
  inline void pub_espresso_t09(float v){ if (espresso_t09_) espresso_t09_->publish_state(v); }
  inline void pub_espresso_t10(float v){ if (espresso_t10_) espresso_t10_->publish_state(v); }
  inline void pub_espresso_t11(float v){ if (espresso_t11_) espresso_t11_->publish_state(v); }
  inline void pub_espresso_t12(float v){ if (espresso_t12_) espresso_t12_->publish_state(v); }
  inline void pub_espresso_timestamp(const std::string &s){ if (espresso_timestamp_) espresso_timestamp_->publish_state(s); }


  // General setters
  bool log_invalid() const { return log_invalid_; }
  void publish_hex_all(const std::string &hex)  { for (auto *s : sinks_all_)  s->publish_frame_hex(hex); }
  void publish_hex_aqua(const std::string &hex) { for (auto *s : sinks_aqua_) s->publish_frame_hex(hex); }

 private:
  uint32_t enabled_mask_{0};
  enum class RxState { SEEK, COLLECT };
  RxState rx_state_{RxState::SEEK};

  // current in-progress frame buffer and target size when known
  std::vector<uint8_t> cur_;
  size_t need_total_{0};  // 0 => unknown yet

  // how many frames we’ll cut per loop() call (keeps latency low)
  static constexpr uint8_t kMaxFramesPerLoop = 3;

  // compute expected total size from partial header; SIZE_MAX => hard desync
  size_t expect_total_if_known_(const std::vector<uint8_t>& v) const;
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

  // decoder instances
  EspressoDecoder *espresso_{nullptr};
  // ESPRESSO sensor pointers
  sensor::Sensor *espresso_t01_{nullptr};
  sensor::Sensor *espresso_t02_{nullptr};
  sensor::Sensor *espresso_t03_{nullptr};
  sensor::Sensor *espresso_t04_{nullptr};
  sensor::Sensor *espresso_t05_{nullptr};
  sensor::Sensor *espresso_pk_{nullptr};
  sensor::Sensor *espresso_t06_{nullptr};
  sensor::Sensor *espresso_t07_{nullptr};
  sensor::Sensor *espresso_t08_{nullptr};
  sensor::Sensor *espresso_t09_{nullptr};
  sensor::Sensor *espresso_t10_{nullptr};
  sensor::Sensor *espresso_t11_{nullptr};
  sensor::Sensor *espresso_t12_{nullptr};
  text_sensor::TextSensor *espresso_timestamp_{nullptr};
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

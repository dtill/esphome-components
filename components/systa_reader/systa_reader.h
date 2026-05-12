#pragma once
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include <deque>
#include <string>
#include <vector>

namespace esphome {
namespace systa_reader {

class AquaDecoder;
class Aqua2Decoder;
class ModulaDecoder;
class EspressoDecoder;
class Palletti2Decoder;
class ModulaDecoder;
class EspressoDecoder;
class Palletti2Decoder;
class CompactDecoder;
class ComfortDecoder;
class SystaReaderTextSink; // forward

class SystaReader : public uart::UARTDevice, public Component {
public:
  // 32-bit space for future devices (up to 32 flags)
  static constexpr uint32_t DEV_AQUA = (1u << 0);
  static constexpr uint32_t DEV_AQUA_II = (1u << 1);
  static constexpr uint32_t DEV_MODULA = (1u << 2);
  static constexpr uint32_t DEV_ESPRESSO = (1u << 3);
  static constexpr uint32_t DEV_SOLAR = (1u << 4);
  static constexpr uint32_t DEV_COMPACT = (1u << 5);
  static constexpr uint32_t DEV_PALLETTI_II = (1u << 6);
  static constexpr uint32_t DEV_COMFORT = (1u << 7);
  // reserve more bits for future devices:
  // static constexpr uint32_t DEV_FOO   = (1u << 4);
  // static constexpr uint32_t DEV_BAR   = (1u << 5);
  // ...

  // TEST-Daten aus YAML
  void set_test_data_frames(const std::vector<std::string> &v) {
    test_data_hex_ = v;
  }

  // config
  void set_log_invalid(bool v) { log_invalid_ = v; }

  // lifecycle
  void setup() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void set_enabled_mask(uint32_t m) { enabled_mask_ = m; }

  // raw sinks
  class HexSink {
  public:
    virtual void publish_frame_hex(const std::string &hex) = 0;
    virtual ~HexSink() = default;
  };
  void add_sink_all(HexSink *s) { sinks_all_.push_back(s); }
  void add_sink_aqua(HexSink *s) { sinks_aqua_.push_back(s); }
  void add_sink_aqua_ii(HexSink *s) { sinks_aqua_ii_.push_back(s); }
  void add_sink_palletti_ii(HexSink *s) { sinks_palletti_ii_.push_back(s); }
  void add_sink_comfort(HexSink *s) { sinks_comfort_.push_back(s); }

  friend class ModulaDecoder;
  friend class AquaDecoder;
  friend class Aqua2Decoder;
  friend class EspressoDecoder;
  friend class Palletti2Decoder;
  friend class CompactDecoder;
  friend class ComfortDecoder;

  // setters (werden von Subplatforms aufgerufen)

  // AQUA numeric
  void set_aqua_tsa_sensor(sensor::Sensor *s) { aqua_tsa_ = s; }
  void set_aqua_tse_sensor(sensor::Sensor *s) { aqua_tse_ = s; }
  void set_aqua_twu_sensor(sensor::Sensor *s) { aqua_twu_ = s; }
  void set_aqua_tw2_sensor(sensor::Sensor *s) { aqua_tw2_ = s; }
  void set_aqua_pwm_sensor(sensor::Sensor *s) { aqua_pwm_ = s; }
  void set_aqua_var1_sensor(sensor::Sensor *s) { aqua_var1_ = s; }
  void set_aqua_stat_sensor(sensor::Sensor *s) { aqua_stat_ = s; }
  void set_aqua_sol_sensor(sensor::Sensor *s) { aqua_sol_ = s; }
  void set_aqua_tag_sensor(sensor::Sensor *s) { aqua_tag_ = s; }
  void set_aqua_ges_sensor(sensor::Sensor *s) { aqua_ges_ = s; }
  void set_aqua_status_code_sensor(sensor::Sensor *s) { aqua_status_code_ = s; }
  // AQUA text
  void set_aqua_status_text_sensor(text_sensor::TextSensor *t) {aqua_status_text_ = t;}
  void set_aqua_timestamp_text_sensor(text_sensor::TextSensor *t) { aqua_timestamp_ = t;}
  // Aqua-Display
  void set_aqua_display_text_sensor(text_sensor::TextSensor *t) {aqua_display_text_ = t;}
  inline void pub_aqua_display_text(const std::string &s) {if (aqua_display_text_)aqua_display_text_->publish_state(s);}
  // Aqua firmware version (announced periodically via FD 05 AA 0B M m p chk)
  void set_aqua_fw_version_text_sensor(text_sensor::TextSensor *t) { aqua_fw_version_ = t; }
  inline void pub_aqua_fw_version(const std::string &s) { if (aqua_fw_version_) aqua_fw_version_->publish_state(s); }

  // publish helpers
  inline void pub_aqua_tsa(float v) { if (aqua_tsa_)    aqua_tsa_->publish_state(v);}
  inline void pub_aqua_tse(float v) { if (aqua_tse_)    aqua_tse_->publish_state(v);}
  inline void pub_aqua_twu(float v) { if (aqua_twu_)    aqua_twu_->publish_state(v);}
  inline void pub_aqua_tw2(float v) { if (aqua_tw2_)    aqua_tw2_->publish_state(v);}
  inline void pub_aqua_pwm(int v)   { if (aqua_pwm_)    aqua_pwm_->publish_state(v);}
  inline void pub_aqua_var1(int v)  { if (aqua_var1_)   aqua_var1_->publish_state(v);}
  inline void pub_aqua_stat(int v)  { if (aqua_stat_)   aqua_stat_->publish_state(v);}
  inline void pub_aqua_sol(float v) { if (aqua_sol_)    aqua_sol_->publish_state(v);}
  inline void pub_aqua_tag(float v) { if (aqua_tag_)    aqua_tag_->publish_state(v);}
  inline void pub_aqua_ges(float v) { if (aqua_ges_)    aqua_ges_->publish_state(v);}
  inline void pub_aqua_status_code(int v)   {if (aqua_status_code_) aqua_status_code_->publish_state(v);}
  inline void pub_aqua_status_text(const std::string &s) { if (aqua_status_text_) aqua_status_text_->publish_state(s);}
  inline void pub_aqua_timestamp(const std::string &s) {if (aqua_timestamp_)aqua_timestamp_->publish_state(s);}

  // AQUA_II numeric
  void set_aqua_ii_tsa_sensor(sensor::Sensor *s) { aqua_ii_tsa_ = s; }
  void set_aqua_ii_twu_sensor(sensor::Sensor *s) { aqua_ii_twu_ = s; }
  void set_aqua_ii_tsv_sensor(sensor::Sensor *s) { aqua_ii_tsv_ = s; }
  void set_aqua_ii_tam_sensor(sensor::Sensor *s) { aqua_ii_tam_ = s; }
  void set_aqua_ii_tse_sensor(sensor::Sensor *s) { aqua_ii_tse_ = s; }
  void set_aqua_ii_dfl_sensor(sensor::Sensor *s) { aqua_ii_dfl_ = s; }
  void set_aqua_ii_pwm_sensor(sensor::Sensor *s) { aqua_ii_pwm_ = s; }
  void set_aqua_ii_koll_lstg_sensor(sensor::Sensor *s) {
    aqua_ii_koll_lstg_ = s;
  }
  void set_aqua_ii_tag_sensor(sensor::Sensor *s) { aqua_ii_tag_ = s; }
  void set_aqua_ii_ges_sensor(sensor::Sensor *s) { aqua_ii_ges_ = s; }
  void set_aqua_ii_tsa1_sensor(sensor::Sensor *s) { aqua_ii_tsa1_ = s; }
  void set_aqua_ii_tsa2_sensor(sensor::Sensor *s) { aqua_ii_tsa2_ = s; }
  void set_aqua_ii_tam2_sensor(sensor::Sensor *s) { aqua_ii_tam2_ = s; }
  void set_aqua_ii_status_code_sensor(sensor::Sensor *s) {
    aqua_ii_status_code_ = s;
  }
  // AQUA_II text
  void set_aqua_ii_status_text_sensor(text_sensor::TextSensor *t)       {aqua_ii_status_text_ = t;}
  void set_aqua_ii_timestamp_text_sensor(text_sensor::TextSensor *t)    {aqua_ii_timestamp_ = t;}
  // publish helpers
  inline void pub_aqua_ii_tsa(float v) { if (aqua_ii_tsa_)  aqua_ii_tsa_->publish_state(v);}
  inline void pub_aqua_ii_twu(float v) { if (aqua_ii_twu_)  aqua_ii_twu_->publish_state(v);}
  inline void pub_aqua_ii_tsv(float v) { if (aqua_ii_tsv_)  aqua_ii_tsv_->publish_state(v);}
  inline void pub_aqua_ii_tam(float v) { if (aqua_ii_tam_)  aqua_ii_tam_->publish_state(v);}
  inline void pub_aqua_ii_tse(float v) { if (aqua_ii_tse_)  aqua_ii_tse_->publish_state(v);}
  inline void pub_aqua_ii_dfl(float v) { if (aqua_ii_dfl_)  aqua_ii_dfl_->publish_state(v);}
  inline void pub_aqua_ii_pwm(float v) { if (aqua_ii_pwm_)  aqua_ii_pwm_->publish_state(v);}
  inline void pub_aqua_ii_koll_lstg(float v) { if (aqua_ii_koll_lstg_) aqua_ii_koll_lstg_->publish_state(v);}
  inline void pub_aqua_ii_tag(float v) { if (aqua_ii_tag_) aqua_ii_tag_->publish_state(v);}
  inline void pub_aqua_ii_ges(float v) { if (aqua_ii_ges_) aqua_ii_ges_->publish_state(v);}
  inline void pub_aqua_ii_tsa1(float v) { if (aqua_ii_tsa1_) aqua_ii_tsa1_->publish_state(v);}
  inline void pub_aqua_ii_tsa2(float v) { if (aqua_ii_tsa2_) aqua_ii_tsa2_->publish_state(v);}
  inline void pub_aqua_ii_tam2(float v) { if (aqua_ii_tam2_) aqua_ii_tam2_->publish_state(v);}
  inline void pub_aqua_ii_status_code(float v) { if (aqua_ii_status_code_) aqua_ii_status_code_->publish_state(v);}
  inline void pub_aqua_ii_status_text(const std::string &s) { if (aqua_ii_status_text_) aqua_ii_status_text_->publish_state(s);}
  inline void pub_aqua_ii_timestamp(const std::string &s) { if (aqua_ii_timestamp_) aqua_ii_timestamp_->publish_state(s);}

  // MODULA setters
  void set_modula_ta_sensor(sensor::Sensor *s) { modula_ta_ = s; }
  void set_modula_two_sensor(sensor::Sensor *s) { modula_two_ = s; }
  void set_modula_tbv_sensor(sensor::Sensor *s) { modula_tbv_ = s; }
  void set_modula_tbr_sensor(sensor::Sensor *s) { modula_tbr_ = s; }
  void set_modula_tv_sensor(sensor::Sensor *s) { modula_tv_ = s; }
  void set_modula_tv2_sensor(sensor::Sensor *s) { modula_tv2_ = s; }
  void set_modula_tr_sensor(sensor::Sensor *s) { modula_tr_ = s; }
  void set_modula_tr2_sensor(sensor::Sensor *s) { modula_tr2_ = s; }
  void set_modula_tpo_sensor(sensor::Sensor *s) { modula_tpo_ = s; }
  void set_modula_tpu_sensor(sensor::Sensor *s) { modula_tpu_ = s; }
  void set_modula_tzr_sensor(sensor::Sensor *s) { modula_tzr_ = s; }
  void set_modula_timestamp_text_sensor(text_sensor::TextSensor *t) {modula_timestamp_ = t;}
  // MODULA publish helpers
  inline void pub_modula_ta(float v) { if (modula_ta_) modula_ta_->publish_state(v);}
  inline void pub_modula_two(float v) { if (modula_two_) modula_two_->publish_state(v);}
  inline void pub_modula_tbv(float v) { if (modula_tbv_) modula_tbv_->publish_state(v);}
  inline void pub_modula_tbr(float v) { if (modula_tbr_) modula_tbr_->publish_state(v);}
  inline void pub_modula_tv(float v) { if (modula_tv_) modula_tv_->publish_state(v);}
  inline void pub_modula_tv2(float v) { if (modula_tv2_) modula_tv2_->publish_state(v);}
  inline void pub_modula_tr(float v) { if (modula_tr_) modula_tr_->publish_state(v);}
  inline void pub_modula_tr2(float v) { if (modula_tr2_) modula_tr2_->publish_state(v);}
  inline void pub_modula_tpo(float v) { if (modula_tpo_) modula_tpo_->publish_state(v);}
  inline void pub_modula_tpu(float v) { if (modula_tpu_) modula_tpu_->publish_state(v);}
  inline void pub_modula_tzr(float v) { if (modula_tzr_) modula_tzr_->publish_state(v);}
  inline void pub_modula_timestamp(const std::string &s) { if (modula_timestamp_) modula_timestamp_->publish_state(s);}

  // ESPRESSO Setter
  void set_espresso_ta_sensor(sensor::Sensor *s) { espresso_ta_ = s; }
  void set_espresso_two_sensor(sensor::Sensor *s) { espresso_two_ = s; }
  void set_espresso_fa_tv_sensor(sensor::Sensor *s) { espresso_fa_tv_ = s; }
  void set_espresso_fa_tr_sensor(sensor::Sensor *s) { espresso_fa_tr_ = s; }
  void set_espresso_hk1_ti_sensor(sensor::Sensor *s) { espresso_hk1_ti_ = s; }
  void set_espresso_hk2_ti2_sensor(sensor::Sensor *s) { espresso_hk2_ti2_ = s; }
  void set_espresso_hk1_tv_sensor(sensor::Sensor *s) { espresso_hk1_tv_ = s; }
  void set_espresso_hk2_tv2_sensor(sensor::Sensor *s) { espresso_hk2_tv2_ = s; }
  void set_espresso_hk1_tr_sensor(sensor::Sensor *s) { espresso_hk1_tr_ = s; }
  void set_espresso_hk2_tr2_sensor(sensor::Sensor *s) { espresso_hk2_tr2_ = s; }
  void set_espresso_tpo_sensor(sensor::Sensor *s) { espresso_tpo_ = s; }
  void set_espresso_tpu_sensor(sensor::Sensor *s) { espresso_tpu_ = s; }
  void set_espresso_tzr_sensor(sensor::Sensor *s) { espresso_tzr_ = s; }
  void set_espresso_pk_sensor(sensor::Sensor *s) { espresso_pk_ = s; }
  void set_espresso_hk1_phk_sensor(sensor::Sensor *s) { espresso_hk1_phk_ = s; }
  void set_espresso_hk2_phk2_sensor(sensor::Sensor *s) { espresso_hk2_phk2_ = s;}
  void set_espresso_timestamp_text_sensor(text_sensor::TextSensor *t) { espresso_timestamp_ = t;}
  // ESPRESSO: private Publisher
  inline void pub_espresso_ta(float v) { if (espresso_ta_) espresso_ta_->publish_state(v);}
  inline void pub_espresso_two(float v) { if (espresso_two_) espresso_two_->publish_state(v);}
  inline void pub_espresso_fa_tv(float v) { if (espresso_fa_tv_) espresso_fa_tv_->publish_state(v);}
  inline void pub_espresso_fa_tr(float v) { if (espresso_fa_tr_) espresso_fa_tr_->publish_state(v);}
  inline void pub_espresso_hk1_ti(float v) { if (espresso_hk1_ti_) espresso_hk1_ti_->publish_state(v);}
  inline void pub_espresso_hk2_ti2(float v) { if (espresso_hk2_ti2_) espresso_hk2_ti2_->publish_state(v);}
  inline void pub_espresso_hk1_tv(float v) { if (espresso_hk1_tv_) espresso_hk1_tv_->publish_state(v);}
  inline void pub_espresso_hk2_tv2(float v) { if (espresso_hk2_tv2_) espresso_hk2_tv2_->publish_state(v);}
  inline void pub_espresso_hk1_tr(float v) { if (espresso_hk1_tr_) espresso_hk1_tr_->publish_state(v);}
  inline void pub_espresso_hk2_tr2(float v) { if (espresso_hk2_tr2_) espresso_hk2_tr2_->publish_state(v);}
  inline void pub_espresso_tpo(float v) { if (espresso_tpo_) espresso_tpo_->publish_state(v);}
  inline void pub_espresso_tpu(float v) { if (espresso_tpu_) espresso_tpu_->publish_state(v);}
  inline void pub_espresso_tzr(float v) { if (espresso_tzr_) espresso_tzr_->publish_state(v);}
  inline void pub_espresso_pk(float v) { if (espresso_pk_) espresso_pk_->publish_state(v);}
  inline void pub_espresso_hk1_phk(float v) { if (espresso_hk1_phk_) espresso_hk1_phk_->publish_state(v);}
  inline void pub_espresso_hk2_phk2(float v) { if (espresso_hk2_phk2_) espresso_hk2_phk2_->publish_state(v);}
  inline void pub_espresso_timestamp(const std::string &s) { if (espresso_timestamp_) espresso_timestamp_->publish_state(s);}

  // PALLETTI_II Setter
  // Bedienteil-Display
  void set_palletti_ii_display_text_sensor(text_sensor::TextSensor *t) { palletti_ii_display_text_ = t;
  }
  inline void pub_palletti_ii_display_text(const std::string &s) {
    if (palletti_ii_display_text_)
      palletti_ii_display_text_->publish_state(s);
  }

  void set_palletti_ii_ta_sensor(sensor::Sensor *s) { palletti_ii_ta_ = s; }
  void set_palletti_ii_two_sensor(sensor::Sensor *s) { palletti_ii_two_ = s; }
  void set_palletti_ii_fa_tv_sensor(sensor::Sensor *s) {
    palletti_ii_fa_tv_ = s;
  }
  void set_palletti_ii_fa_tr_sensor(sensor::Sensor *s) {
    palletti_ii_fa_tr_ = s;
  }
  void set_palletti_ii_hk1_ti_sensor(sensor::Sensor *s) {
    palletti_ii_hk1_ti_ = s;
  }
  void set_palletti_ii_hk2_ti2_sensor(sensor::Sensor *s) {
    palletti_ii_hk2_ti2_ = s;
  }
  void set_palletti_ii_hk1_tv_sensor(sensor::Sensor *s) {
    palletti_ii_hk1_tv_ = s;
  }
  void set_palletti_ii_hk2_tv2_sensor(sensor::Sensor *s) {
    palletti_ii_hk2_tv2_ = s;
  }
  void set_palletti_ii_hk1_tr_sensor(sensor::Sensor *s) {
    palletti_ii_hk1_tr_ = s;
  }
  void set_palletti_ii_hk2_tr2_sensor(sensor::Sensor *s) {
    palletti_ii_hk2_tr2_ = s;
  }
  void set_palletti_ii_tpo_sensor(sensor::Sensor *s) { palletti_ii_tpo_ = s; }
  void set_palletti_ii_tpu_sensor(sensor::Sensor *s) { palletti_ii_tpu_ = s; }
  void set_palletti_ii_tzr_sensor(sensor::Sensor *s) { palletti_ii_tzr_ = s; }
  void set_palletti_ii_pk_sensor(sensor::Sensor *s) { palletti_ii_pk_ = s; }
  void set_palletti_ii_hk1_phk_sensor(sensor::Sensor *s) {
    palletti_ii_hk1_phk_ = s;
  }
  void set_palletti_ii_hk2_phk2_sensor(sensor::Sensor *s) {
    palletti_ii_hk2_phk2_ = s;
  }
  void set_palletti_ii_timestamp_text_sensor(text_sensor::TextSensor *t) {
    palletti_ii_timestamp_ = t;
  }
  // PALLETTI-II: private Publisher
  inline void pub_palletti_ii_ta(float v) {
    if (palletti_ii_ta_)
      palletti_ii_ta_->publish_state(v);
  }
  inline void pub_palletti_ii_two(float v) {
    if (palletti_ii_two_)
      palletti_ii_two_->publish_state(v);
  }
  inline void pub_palletti_ii_fa_tv(float v) {
    if (palletti_ii_fa_tv_)
      palletti_ii_fa_tv_->publish_state(v);
  }
  inline void pub_palletti_ii_fa_tr(float v) {
    if (palletti_ii_fa_tr_)
      palletti_ii_fa_tr_->publish_state(v);
  }
  inline void pub_palletti_ii_hk1_ti(float v) {
    if (palletti_ii_hk1_ti_)
      palletti_ii_hk1_ti_->publish_state(v);
  }
  inline void pub_palletti_ii_hk2_ti2(float v) {
    if (palletti_ii_hk2_ti2_)
      palletti_ii_hk2_ti2_->publish_state(v);
  }
  inline void pub_palletti_ii_hk1_tv(float v) {
    if (palletti_ii_hk1_tv_)
      palletti_ii_hk1_tv_->publish_state(v);
  }
  inline void pub_palletti_ii_hk2_tv2(float v) {
    if (palletti_ii_hk2_tv2_)
      palletti_ii_hk2_tv2_->publish_state(v);
  }
  inline void pub_palletti_ii_hk1_tr(float v) {
    if (palletti_ii_hk1_tr_)
      palletti_ii_hk1_tr_->publish_state(v);
  }
  inline void pub_palletti_ii_hk2_tr2(float v) {
    if (palletti_ii_hk2_tr2_)
      palletti_ii_hk2_tr2_->publish_state(v);
  }
  inline void pub_palletti_ii_tpo(float v) {
    if (palletti_ii_tpo_)
      palletti_ii_tpo_->publish_state(v);
  }
  inline void pub_palletti_ii_tpu(float v) {
    if (palletti_ii_tpu_)
      palletti_ii_tpu_->publish_state(v);
  }
  inline void pub_palletti_ii_tzr(float v) {
    if (palletti_ii_tzr_)
      palletti_ii_tzr_->publish_state(v);
  }
  inline void pub_palletti_ii_pk(float v) {
    if (palletti_ii_pk_)
      palletti_ii_pk_->publish_state(v);
  }
  inline void pub_palletti_ii_hk1_phk(float v) {
    if (palletti_ii_hk1_phk_)
      palletti_ii_hk1_phk_->publish_state(v);
  }
  inline void pub_palletti_ii_hk2_phk2(float v) {
    if (palletti_ii_hk2_phk2_)
      palletti_ii_hk2_phk2_->publish_state(v);
  }
  inline void pub_palletti_ii_timestamp(const std::string &s) {
    if (palletti_ii_timestamp_)
      palletti_ii_timestamp_->publish_state(s);
  }

  // COMPACT Setter
  void set_compact_ta_sensor(sensor::Sensor *s) { compact_ta_ = s; }
  void set_compact_two_sensor(sensor::Sensor *s) { compact_two_ = s; }
  void set_compact_fa_tv_sensor(sensor::Sensor *s) { compact_fa_tv_ = s; }
  void set_compact_fa_tr_sensor(sensor::Sensor *s) { compact_fa_tr_ = s; }
  void set_compact_ti_sensor(sensor::Sensor *s) { compact_ti_ = s; }
  void set_compact_ti_s_sensor(sensor::Sensor *s) { compact_ti_s_ = s; }
  void set_compact_tv_s_sensor(sensor::Sensor *s) { compact_tv_s_ = s; }
  void set_compact_two_s_sensor(sensor::Sensor *s) { compact_two_s_ = s; }
  void set_compact_status_code_sensor(sensor::Sensor *s) { compact_status_code_ = s;}
  void set_compact_timestamp_text_sensor(text_sensor::TextSensor *t) { compact_timestamp_ = t;}

  // COMPACT: private Publisher
  inline void pub_compact_ta(float v) {
    if (compact_ta_)
      compact_ta_->publish_state(v);
  }
  inline void pub_compact_two(float v) {
    if (compact_two_)
      compact_two_->publish_state(v);
  }
  inline void pub_compact_fa_tv(float v) {
    if (compact_fa_tv_)
      compact_fa_tv_->publish_state(v);
  }
  inline void pub_compact_fa_tr(float v) {
    if (compact_fa_tr_)
      compact_fa_tr_->publish_state(v);
  }
  inline void pub_compact_ti(float v) {
    if (compact_ti_)
      compact_ti_->publish_state(v);
  }
  inline void pub_compact_ti_s(float v) {
    if (compact_ti_s_)
      compact_ti_s_->publish_state(v);
  }
  inline void pub_compact_tv_s(float v) {
    if (compact_tv_s_)
      compact_tv_s_->publish_state(v);
  }
  inline void pub_compact_two_s(float v) {
    if (compact_two_s_)
      compact_two_s_->publish_state(v);
  }
  inline void pub_compact_status_code(float v) {
    if (compact_status_code_)
      compact_status_code_->publish_state(v);
  }
  inline void pub_compact_timestamp(const std::string &s) {
    if (compact_timestamp_)
      compact_timestamp_->publish_state(s);
  }

  // COMFORT Setter
  void set_comfort_ti_s_sensor(sensor::Sensor *s) { comfort_ti_s_ = s; }
  void set_comfort_ti2_s_sensor(sensor::Sensor *s) { comfort_ti2_s_ = s; }
  void set_comfort_tv_s_sensor(sensor::Sensor *s) { comfort_tv_s_ = s; }
  void set_comfort_tv2_s_sensor(sensor::Sensor *s) { comfort_tv2_s_ = s; }
  void set_comfort_tw_s_sensor(sensor::Sensor *s) { comfort_tw_s_ = s; }
  void set_comfort_tp_s_sensor(sensor::Sensor *s) { comfort_tp_s_ = s; }
  void set_comfort_bst_sensor(sensor::Sensor *s) { comfort_bst_ = s; }
  void set_comfort_kst_sensor(sensor::Sensor *s) { comfort_kst_ = s; }
  void set_comfort_sens_sensor(sensor::Sensor *s) { comfort_sens_ = s; }
  void set_comfort_ba1_sensor(sensor::Sensor *s) { comfort_ba1_ = s; }
  void set_comfort_niv1_sensor(sensor::Sensor *s) { comfort_niv1_ = s; }
  void set_comfort_ba2_sensor(sensor::Sensor *s) { comfort_ba2_ = s; }
  void set_comfort_niv2_sensor(sensor::Sensor *s) { comfort_niv2_ = s; }
  void set_comfort_phk1_sensor(sensor::Sensor *s) { comfort_phk1_ = s; }
  void set_comfort_phk2_sensor(sensor::Sensor *s) { comfort_phk2_ = s; }
  void set_comfort_pkes_sensor(sensor::Sensor *s) { comfort_pkes_ = s; }
  void set_comfort_boiler_err_text_sensor(text_sensor::TextSensor *t) { comfort_boiler_err_text_sensor_ = t;}
  void set_comfort_fw_version_text_sensor(text_sensor::TextSensor *t) { comfort_fw_version_ = t; }
  inline void pub_comfort_fw_version(const std::string &s) { if (comfort_fw_version_) comfort_fw_version_->publish_state(s); }

  // COMFORT Status Bits
  void set_comfort_stat_phk_binary_sensor(binary_sensor::BinarySensor *s) { comfort_stat_phk_ = s;}
  void set_comfort_stat_phk2_binary_sensor(binary_sensor::BinarySensor *s) { comfort_stat_phk2_ = s;}
  void set_comfort_stat_pk_binary_sensor(binary_sensor::BinarySensor *s) { comfort_stat_pk_ = s;}
  void set_comfort_stat_m1_open_binary_sensor(binary_sensor::BinarySensor *s) { comfort_stat_m1_open_ = s;}
  void set_comfort_stat_m1_close_binary_sensor(binary_sensor::BinarySensor *s) { comfort_stat_m1_close_ = s;}
  void set_comfort_stat_m2_open_binary_sensor(binary_sensor::BinarySensor *s) { comfort_stat_m2_open_ = s;}
  void set_comfort_stat_m2_close_binary_sensor(binary_sensor::BinarySensor *s) { comfort_stat_m2_close_ = s;}
  void set_comfort_stat_ulv_binary_sensor(binary_sensor::BinarySensor *s) { comfort_stat_ulv_ = s;}
  void set_comfort_stat_pzi_binary_sensor(binary_sensor::BinarySensor *s) { comfort_stat_pzi_ = s;}
  void set_comfort_stat_b1_binary_sensor(binary_sensor::BinarySensor *s) { comfort_stat_b1_ = s;}
  void set_comfort_stat_taster_binary_sensor(binary_sensor::BinarySensor *s) { comfort_stat_taster_ = s;}
  void set_comfort_stat_lon_binary_sensor(binary_sensor::BinarySensor *s) { comfort_stat_lon_ = s;}
  void set_comfort_stat_ot_binary_sensor(binary_sensor::BinarySensor *s) { comfort_stat_ot_ = s;}

  // COMFORT: private Publisher
  inline void pub_comfort_ti_s(float v) { if (comfort_ti_s_) comfort_ti_s_->publish_state(v);}
  inline void pub_comfort_ti2_s(float v) { if (comfort_ti2_s_) comfort_ti2_s_->publish_state(v);}
  inline void pub_comfort_tv_s(float v) { if (comfort_tv_s_) comfort_tv_s_->publish_state(v);}
  inline void pub_comfort_tv2_s(float v) { if (comfort_tv2_s_) comfort_tv2_s_->publish_state(v);}
  inline void pub_comfort_tw_s(float v) { if (comfort_tw_s_) comfort_tw_s_->publish_state(v);}
  inline void pub_comfort_tp_s(float v) { if (comfort_tp_s_) comfort_tp_s_->publish_state(v);}
  inline void pub_comfort_bst(int v) { if (comfort_bst_) comfort_bst_->publish_state(v);}
  inline void pub_comfort_kst(int v) { if (comfort_kst_) comfort_kst_->publish_state(v);}
  inline void pub_comfort_sens(int v) { if (comfort_sens_) comfort_sens_->publish_state(v);}
  inline void pub_comfort_ba1(int v) { if (comfort_ba1_) comfort_ba1_->publish_state(v);}
  inline void pub_comfort_niv1(int v) { if (comfort_niv1_) comfort_niv1_->publish_state(v);}
  inline void pub_comfort_ba2(int v) { if (comfort_ba2_) comfort_ba2_->publish_state(v);}
  inline void pub_comfort_niv2(int v) { if (comfort_niv2_) comfort_niv2_->publish_state(v);}
  inline void pub_comfort_phk1(int v) { if (comfort_phk1_) comfort_phk1_->publish_state(v);}
  inline void pub_comfort_phk2(int v) { if (comfort_phk2_) comfort_phk2_->publish_state(v);}
  inline void pub_comfort_pkes(int v) { if (comfort_pkes_) comfort_pkes_->publish_state(v);}
  inline void pub_comfort_boiler_err_text(const std::string &s) { if (comfort_boiler_err_text_sensor_) comfort_boiler_err_text_sensor_->publish_state(s);}

  // COMFORT Status Bits Publishers
  inline void pub_comfort_stat_phk(bool v) { if (comfort_stat_phk_) comfort_stat_phk_->publish_state(v);}
  inline void pub_comfort_stat_phk2(bool v) { if (comfort_stat_phk2_) comfort_stat_phk2_->publish_state(v);}
  inline void pub_comfort_stat_pk(bool v) { if (comfort_stat_pk_) comfort_stat_pk_->publish_state(v);}
  inline void pub_comfort_stat_m1_open(bool v) { if (comfort_stat_m1_open_) comfort_stat_m1_open_->publish_state(v);}
  inline void pub_comfort_stat_m1_close(bool v) { if (comfort_stat_m1_close_) comfort_stat_m1_close_->publish_state(v);}
  inline void pub_comfort_stat_m2_open(bool v) { if (comfort_stat_m2_open_) comfort_stat_m2_open_->publish_state(v);}
  inline void pub_comfort_stat_m2_close(bool v) { if (comfort_stat_m2_close_) comfort_stat_m2_close_->publish_state(v);}
  inline void pub_comfort_stat_ulv(bool v) { if (comfort_stat_ulv_) comfort_stat_ulv_->publish_state(v);}
  inline void pub_comfort_stat_pzi(bool v) { if (comfort_stat_pzi_) comfort_stat_pzi_->publish_state(v);}
  inline void pub_comfort_stat_b1(bool v) { if (comfort_stat_b1_) comfort_stat_b1_->publish_state(v);}
  inline void pub_comfort_stat_taster(bool v) { if (comfort_stat_taster_) comfort_stat_taster_->publish_state(v);}
  inline void pub_comfort_stat_lon(bool v) { if (comfort_stat_lon_) comfort_stat_lon_->publish_state(v);}
  inline void pub_comfort_stat_ot(bool v) { if (comfort_stat_ot_) comfort_stat_ot_->publish_state(v);}

  // General setters
  bool log_invalid() const { return log_invalid_; }
  void publish_hex_all(const std::string &hex) {
    for (auto *s : sinks_all_)
      s->publish_frame_hex(hex);
  }
  void publish_hex_aqua(const std::string &hex) {
    for (auto *s : sinks_aqua_)
      s->publish_frame_hex(hex);
  }
  void publish_hex_aqua_ii(const std::string &hex) {
    for (auto *s : sinks_aqua_ii_)
      s->publish_frame_hex(hex);
  }
  void publish_hex_palletti_ii(const std::string &hex) {
    for (auto *s : sinks_palletti_ii_)
      s->publish_frame_hex(hex);
  }
  void publish_hex_comfort(const std::string &hex) {
    for (auto *s : sinks_comfort_)
      s->publish_frame_hex(hex);
  }

private:
  uint32_t enabled_mask_{0};
  enum class RxState { SEEK, COLLECT };
  RxState rx_state_{RxState::SEEK};

  // current in-progress frame buffer and target size when known
  std::vector<uint8_t> cur_;
  size_t need_total_{0}; // 0 => unknown yet

  // bytes consumed in SEEK that didn't match a known sync — flushed via
  // ESP_LOGV when we either sync up, hit the cap, or get a hard desync from a
  // rejected partial frame. Lets unknown sender frames show up in the log.
  std::vector<uint8_t> skipped_;
  static constexpr size_t kSkippedFlushCap = 256;
  void flush_skipped_(const char *reason);

  // Emit a labeled ESP_LOGV line ("FC HEX:", "Cmd HEX:", "Display HEX:", …)
  // for any frame >= 4 bytes. Used both for live bus traffic and for
  // injected test_data, so test frames are visible even when their target
  // decoder isn't enabled in the YAML.
  void log_frame_hex_(const std::vector<uint8_t> &frame,
                      const std::string &hex);

  // how many frames we’ll cut per loop() call (keeps latency low)
  static constexpr uint8_t kMaxFramesPerLoop = 3;

  // compute expected total size from partial header; SIZE_MAX => hard desync
  size_t expect_total_if_known_(const std::vector<uint8_t> &v) const;
  // parsing
  void process_buffer_();
  bool try_parse_display_frame_();
  bool try_parse_fc_frame_();

  // Test-Injector
  std::vector<std::string> test_data_hex_{};
  uint32_t last_inject_ms_{0};
  static std::vector<uint8_t> hex_to_bytes_(const std::string &hex);
  void inject_test_frames_();

  // kleiner Router für FC-Frames (ruft nur das gewählte Gerät auf)
  void route_fc_frame_to_device_(const std::vector<uint8_t> &frame,
                                 const std::vector<uint8_t> &payload,
                                 const std::string &hex);
  // kleiner Router für 0F-Frames (ruft nur das gewählte Gerät auf)
  void route_display_frame_to_device_(const std::vector<uint8_t> &frame,
                                      const std::vector<uint8_t> &payload,
                                      const std::string &hex);

  // utils
  static uint8_t
  checksum_twos_complement_(const std::vector<uint8_t> &data_wo_last);
  static std::string to_hex_(const std::vector<uint8_t> &buf);

  // state
  std::deque<uint8_t> buf_;
  std::vector<HexSink *> sinks_all_;
  std::vector<HexSink *> sinks_aqua_;
  std::vector<HexSink *> sinks_aqua_ii_;
  std::vector<HexSink *> sinks_palletti_ii_;
  std::vector<HexSink *> sinks_comfort_;
  std::vector<SystaReaderTextSink *> sinks_{};
  bool log_invalid_{true};

  // decoder instances
  AquaDecoder *aqua_{nullptr};
  // AQUA sensors
  sensor::Sensor *aqua_tsa_{nullptr};
  sensor::Sensor *aqua_tse_{nullptr};
  sensor::Sensor *aqua_twu_{nullptr};
  sensor::Sensor *aqua_tw2_{nullptr};
  sensor::Sensor *aqua_pwm_{nullptr};
  sensor::Sensor *aqua_var1_{nullptr};
  sensor::Sensor *aqua_stat_{nullptr};
  sensor::Sensor *aqua_sol_{nullptr};
  sensor::Sensor *aqua_tag_{nullptr};
  sensor::Sensor *aqua_ges_{nullptr};
  sensor::Sensor *aqua_status_code_{nullptr};
  text_sensor::TextSensor *aqua_status_text_{nullptr};
  text_sensor::TextSensor *aqua_timestamp_{nullptr};
  text_sensor::TextSensor *aqua_display_text_{nullptr};
  text_sensor::TextSensor *aqua_fw_version_{nullptr};

  // decoder instances
  Aqua2Decoder *aqua_ii_{nullptr};
  // AQUA-II sensors
  sensor::Sensor *aqua_ii_tsa_{nullptr};
  sensor::Sensor *aqua_ii_twu_{nullptr};
  sensor::Sensor *aqua_ii_tsv_{nullptr};
  sensor::Sensor *aqua_ii_tam_{nullptr};
  sensor::Sensor *aqua_ii_tse_{nullptr};
  sensor::Sensor *aqua_ii_dfl_{nullptr};
  sensor::Sensor *aqua_ii_pwm_{nullptr};
  sensor::Sensor *aqua_ii_koll_lstg_{nullptr};
  sensor::Sensor *aqua_ii_tag_{nullptr};
  sensor::Sensor *aqua_ii_ges_{nullptr};
  sensor::Sensor *aqua_ii_tsa1_{nullptr};
  sensor::Sensor *aqua_ii_tsa2_{nullptr};
  sensor::Sensor *aqua_ii_tam2_{nullptr};
  sensor::Sensor *aqua_ii_status_code_{nullptr};
  text_sensor::TextSensor *aqua_ii_status_text_{nullptr};
  text_sensor::TextSensor *aqua_ii_timestamp_{nullptr};

  // decoder instances
  ModulaDecoder *modula_{nullptr};
  // MODULA sensor pointers
  sensor::Sensor *modula_ta_{nullptr};
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
  sensor::Sensor *espresso_ta_{nullptr};
  sensor::Sensor *espresso_two_{nullptr};
  sensor::Sensor *espresso_fa_tv_{nullptr};
  sensor::Sensor *espresso_fa_tr_{nullptr};
  sensor::Sensor *espresso_hk1_ti_{nullptr};
  sensor::Sensor *espresso_hk2_ti2_{nullptr};
  sensor::Sensor *espresso_hk1_tv_{nullptr};
  sensor::Sensor *espresso_hk2_tv2_{nullptr};
  sensor::Sensor *espresso_hk1_tr_{nullptr};
  sensor::Sensor *espresso_hk2_tr2_{nullptr};
  sensor::Sensor *espresso_tpo_{nullptr};
  sensor::Sensor *espresso_tpu_{nullptr};
  sensor::Sensor *espresso_tzr_{nullptr};
  sensor::Sensor *espresso_pk_{nullptr};
  sensor::Sensor *espresso_hk1_phk_{nullptr};
  sensor::Sensor *espresso_hk2_phk2_{nullptr};
  text_sensor::TextSensor *espresso_timestamp_{nullptr};

  // decoder instances
  Palletti2Decoder *palletti_ii_{nullptr};
  // PALLETTI-II sensor pointers
  sensor::Sensor *palletti_ii_ta_{nullptr};
  sensor::Sensor *palletti_ii_two_{nullptr};
  sensor::Sensor *palletti_ii_fa_tv_{nullptr};
  sensor::Sensor *palletti_ii_fa_tr_{nullptr};
  sensor::Sensor *palletti_ii_hk1_ti_{nullptr};
  sensor::Sensor *palletti_ii_hk2_ti2_{nullptr};
  sensor::Sensor *palletti_ii_hk1_tv_{nullptr};
  sensor::Sensor *palletti_ii_hk2_tv2_{nullptr};
  sensor::Sensor *palletti_ii_hk1_tr_{nullptr};
  sensor::Sensor *palletti_ii_hk2_tr2_{nullptr};
  sensor::Sensor *palletti_ii_tpo_{nullptr};
  sensor::Sensor *palletti_ii_tpu_{nullptr};
  sensor::Sensor *palletti_ii_tzr_{nullptr};
  sensor::Sensor *palletti_ii_pk_{nullptr};
  sensor::Sensor *palletti_ii_hk1_phk_{nullptr};
  sensor::Sensor *palletti_ii_hk2_phk2_{nullptr};
  text_sensor::TextSensor *palletti_ii_timestamp_{nullptr};
  text_sensor::TextSensor *palletti_ii_display_text_{nullptr};

  // decoder instances
  CompactDecoder *compact_{nullptr};
  // COMPACT sensor pointers
  sensor::Sensor *compact_ta_{nullptr};
  sensor::Sensor *compact_two_{nullptr};
  sensor::Sensor *compact_fa_tv_{nullptr};
  sensor::Sensor *compact_fa_tr_{nullptr};
  sensor::Sensor *compact_ti_{nullptr};
  sensor::Sensor *compact_ti_s_{nullptr};
  sensor::Sensor *compact_tv_s_{nullptr};
  sensor::Sensor *compact_two_s_{nullptr};
  sensor::Sensor *compact_status_code_{nullptr};
  text_sensor::TextSensor *compact_timestamp_{nullptr};

  // decoder instances
  ComfortDecoder *comfort_{nullptr};
  // COMFORT sensors
  sensor::Sensor *comfort_ti_s_{nullptr};
  sensor::Sensor *comfort_ti2_s_{nullptr};
  sensor::Sensor *comfort_tv_s_{nullptr};
  sensor::Sensor *comfort_tv2_s_{nullptr};
  sensor::Sensor *comfort_tw_s_{nullptr};
  sensor::Sensor *comfort_tp_s_{nullptr};
  sensor::Sensor *comfort_bst_{nullptr};
  sensor::Sensor *comfort_kst_{nullptr};
  sensor::Sensor *comfort_sens_{nullptr};
  sensor::Sensor *comfort_ba1_{nullptr};
  sensor::Sensor *comfort_niv1_{nullptr};
  sensor::Sensor *comfort_ba2_{nullptr};
  sensor::Sensor *comfort_niv2_{nullptr};
  sensor::Sensor *comfort_phk1_{nullptr};
  sensor::Sensor *comfort_phk2_{nullptr};
  sensor::Sensor *comfort_pkes_{nullptr};
  text_sensor::TextSensor *comfort_boiler_err_text_sensor_{nullptr};
  text_sensor::TextSensor *comfort_fw_version_{nullptr};

  // COMFORT status bit sensors
  binary_sensor::BinarySensor *comfort_stat_phk_{nullptr};
  binary_sensor::BinarySensor *comfort_stat_phk2_{nullptr};
  binary_sensor::BinarySensor *comfort_stat_pk_{nullptr};
  binary_sensor::BinarySensor *comfort_stat_m1_open_{nullptr};
  binary_sensor::BinarySensor *comfort_stat_m1_close_{nullptr};
  binary_sensor::BinarySensor *comfort_stat_m2_open_{nullptr};
  binary_sensor::BinarySensor *comfort_stat_m2_close_{nullptr};
  binary_sensor::BinarySensor *comfort_stat_ulv_{nullptr};
  binary_sensor::BinarySensor *comfort_stat_pzi_{nullptr};
  binary_sensor::BinarySensor *comfort_stat_b1_{nullptr};
  binary_sensor::BinarySensor *comfort_stat_taster_{nullptr};
  binary_sensor::BinarySensor *comfort_stat_lon_{nullptr};
  binary_sensor::BinarySensor *comfort_stat_ot_{nullptr};
};

// Sink-Interface (bestehend)
class SystaReaderTextSink {
public:
  virtual void publish_frame_hex(const std::string &hex) = 0;
  virtual ~SystaReaderTextSink() = default;
};

// concrete text sensor sink
class SystaReaderTextSensor : public text_sensor::TextSensor,
                              public Component,
                              public SystaReader::HexSink {
public:
  void publish_frame_hex(const std::string &hex) override {
    this->publish_state(hex);
  }
};

} // namespace systa_reader
} // namespace esphome

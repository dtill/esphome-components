#include "espresso.h"
#include "esphome/core/log.h"

namespace esphome {
namespace systa_reader {

static const char *const TAG_ESP = "systa_reader.espresso";

void EspressoDecoder::on_fc_frame(const std::vector<uint8_t>& frame,
                                  const std::vector<uint8_t>& payload,
                                  const std::string &/*hex*/) {
  // Nur Espresso-Frames: FC .. 0C 01 ..
  if (frame.size() < 6 || frame[0] != 0xFC || frame[2] != 0x0C || frame[3] != 0x01)
    return;

  auto u16 = [&](int i){ return read_u16_be(frame, i); };

  // Zeitstempel
  uint8_t day    = bcd2dec(payload[0]);
  uint8_t month  = bcd2dec(payload[1]);
  uint8_t minute = bcd2dec(payload[2]);
  uint8_t hour   = bcd2dec(payload[3]);
  char ts[16];
  snprintf(ts, sizeof(ts), "%02d.%02d %02d:%02d", day, month, hour, minute);
  r_.pub_espresso_timestamp(ts);

  // Werte gemäß Vorlage
  float ta = u16(8) / 10.0f;
  float two = u16(10) / 10.0f;
  float fa_tv = u16(12) / 10.0f;
  float fa_tr = u16(14) / 10.0f;
  float hk1_ti = u16(16) / 10.0f;
  float hk2_ti2 = u16(18) / 10.0f;
  float hk1_tv  = u16(20) / 10.0f;
  float hk2_tv2 = u16(22) / 10.0f;
  float hk1_tr = u16(24) / 10.0f;
  float hk2_tr2 = u16(26) / 10.0f;
  float tpo = u16(28) / 10.0f;
  float tpu = u16(30) / 10.0f;
  float tzr = u16(32) / 10.0f;
  float t12 = u16(34) / 10.0f;

  r_.pub_espresso_ta(ta);
  r_.pub_espresso_two(two);
  r_.pub_espresso_fa_tv(fa_tv);
  r_.pub_espresso_fa_tr(fa_tr);
  r_.pub_espresso_ti(hk1_ti);
  r_.pub_espresso_ti2(hk2_ti2);
  r_.pub_espresso_hk1_tv (hk1_tv);
  r_.pub_espresso_hk2_tv2(hk2_tv2);
  r_.pub_espresso_hk1_tr(hk1_tr);
  r_.pub_espresso_hk2_tr2(hk2_tr2);
  r_.pub_espresso_tpo(tpo);
  r_.pub_espresso_tpu(tpu);
  r_.pub_espresso_tzr(tzr);
  r_.pub_espresso_t12(t12);

  ESP_LOGI(TAG_ESP, "ESPRESSO: %s | TA=%.1f TWO=%.1f FA TV=%.1f FA TR=%.1f HK1 TI=%.1f HK2 TI2=%.1f HK1 TV=%.1f HK2 TV2=%.1f HK1 TR=%.1f HK2 TR2=%.1f TPO=%.1f TPU=%.1f TZR=%.1f T12=%.1f",
           ta, two,fa_tv,fa_tr,hk1_ti,hk2_ti2,hk1_tv,hk2_tv2,hk1_tr,hk2_tr2,tpo,tpu,tzr,t12);
}

}  // namespace systa_reader
}  // namespace esphome

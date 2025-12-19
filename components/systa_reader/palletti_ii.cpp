#include "palletti_ii.h"
#include "esphome/core/log.h"

namespace esphome {
namespace systa_reader {

static const char *const TAG_PALLETTI_II = "systa_reader.palletti_ii";

void Palletti2Decoder::on_fc_frame(const std::vector<uint8_t>& frame,
                                  const std::vector<uint8_t>& payload,
                                  const std::string &/*hex*/) {
  // Nur Palletti2-Frames: FC .. 0C 01 ..
  if (frame.size() < 6 || frame[0] != 0xFC || frame[2] != 0x0C || frame[3] != 0x01)
    return;

  auto u16 = [&](int i){ return read_u16_be(frame, i); };
  auto u8  = [&](int i){ return read_u8(frame, i); };

  // Zeitstempel
  uint8_t day    = bcd2dec(payload[0]);
  uint8_t month  = bcd2dec(payload[1]);
  uint8_t minute = bcd2dec(payload[2]);
  uint8_t hour   = bcd2dec(payload[3]);
  char ts[16];
  snprintf(ts, sizeof(ts), "%02d.%02d %02d:%02d", day, month, hour, minute);
  r_.pub_palletti_ii_timestamp(ts);

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
  float pk = u8(34) / 1.0f;
  float hk1_phk = u8(35) / 1.0f;
  float hk2_phk2 = u8(36) / 1.0f;

  r_.pub_palletti_ii_ta(ta);
  r_.pub_palletti_ii_two(two);
  r_.pub_palletti_ii_fa_tv(fa_tv);
  r_.pub_palletti_ii_fa_tr(fa_tr);
  r_.pub_palletti_ii_hk1_ti(hk1_ti);
  r_.pub_palletti_ii_hk2_ti2(hk2_ti2);
  r_.pub_palletti_ii_hk1_tv (hk1_tv);
  r_.pub_palletti_ii_hk2_tv2(hk2_tv2);
  r_.pub_palletti_ii_hk1_tr(hk1_tr);
  r_.pub_palletti_ii_hk2_tr2(hk2_tr2);
  r_.pub_palletti_ii_tpo(tpo);
  r_.pub_palletti_ii_tpu(tpu);
  r_.pub_palletti_ii_tzr(tzr);
  r_.pub_palletti_ii_pk(pk);
  r_.pub_palletti_ii_hk1_phk(hk1_phk);
  r_.pub_palletti_ii_hk2_phk2(hk2_phk2);

  ESP_LOGI(TAG_PALLETTI_II, "PALLETTI-II: TA=%.1f TWO=%.1f FA TV=%.1f FA TR=%.1f HK1 TI=%.1f HK2 TI2=%.1f HK1 TV=%.1f HK2 TV2=%.1f HK1 TR=%.1f HK2 TR2=%.1f TPO=%.1f TPU=%.1f TZR=%.1f PK=%.0f HK1 PHK=%.0f HK2 PHK2=%.0f",
           ta,two,fa_tv,fa_tr,hk1_ti,hk2_ti2,hk1_tv,hk2_tv2,hk1_tr,hk2_tr2,tpo,tpu,tzr,pk,hk1_phk,hk2_phk2);
}

void Palletti2Decoder::on_display_frame(const std::vector<uint8_t>& /*frame*/,
                                   const std::vector<uint8_t>& payload,
                                   const std::string &/*hex*/) {
  // payload ist 32 Byte ASCII (mit evtl. non-printables → '.' ersetzen)
  std::string ascii; ascii.reserve(payload.size());
  for (auto b : payload) ascii += (b >= 32 && b <= 126) ? char(b) : '.';

  if (ascii == last_display_) return;  // unverändert

  const uint32_t now = millis();
  if (last_display_ms_ != 0 && (uint32_t)(now - last_display_ms_) < DISPLAY_MIN_INTERVAL_MS) {
    // innerhalb 5 min: nur leise loggen
    ESP_LOGV(TAG_PALLETTI_II, "Display change suppressed (interval): \"%s\"", ascii.c_str());
    last_display_ = ascii;  // trotzdem aktualisieren, damit nach Ablauf nicht „altes“ kommt
    return;
  }

  ESP_LOGD(TAG_PALLETTI_II, "Display-Text geändert: \"%s\"", ascii.c_str());
  r_.pub_aqua_display_text(ascii);
  last_display_   = ascii;
  last_display_ms_= now;
}

}  // namespace systa_reader
}  // namespace esphome

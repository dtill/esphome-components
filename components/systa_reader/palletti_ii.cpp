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
  auto s16 = [&](int i){ return (int16_t)read_u16_be(frame, i); };
  auto u8  = [&](int i){ return read_u8(frame, i); };

  // 0C01 exists in several lengths. The last data byte is at
  // frame.size() - 2; the byte after it is the checksum. Without this guard
  // a short frame makes pk read the checksum and phk read past the end,
  // which yields plausible-looking but meaningless values.
  const int last_data = (int) frame.size() - 2;
  if (last_data < 33) {
    ESP_LOGW(TAG_PALLETTI_II, "Frame too short: len=%u, need at least 31", frame[1]);
    return;
  }

  // Zeitstempel
  uint8_t day    = bcd2dec(payload[0]);
  uint8_t month  = bcd2dec(payload[1]);
  uint8_t minute = bcd2dec(payload[2]);
  uint8_t hour   = bcd2dec(payload[3]);
  char ts[16];
  snprintf(ts, sizeof(ts), "%02d.%02d %02d:%02d", day, month, hour, minute);
  r_.pub_palletti_ii_timestamp(ts);

  // Werte gemäß Vorlage
  float ta = r_.temp_s16(TAG_PALLETTI_II, "PALLETTI-II TA", s16(8));
  float two = r_.temp_u16(TAG_PALLETTI_II, "PALLETTI-II TWO", u16(10));
  float fa_tv = r_.temp_u16(TAG_PALLETTI_II, "PALLETTI-II FA TV", u16(12));
  float fa_tr = r_.temp_u16(TAG_PALLETTI_II, "PALLETTI-II FA TR", u16(14));
  float hk1_ti = r_.temp_u16(TAG_PALLETTI_II, "PALLETTI-II HK1 TI", u16(16));
  float hk2_ti2 = r_.temp_u16(TAG_PALLETTI_II, "PALLETTI-II HK2 TI2", u16(18));
  float hk1_tv  = r_.temp_u16(TAG_PALLETTI_II, "PALLETTI-II HK1 TV", u16(20));
  float hk2_tv2 = r_.temp_u16(TAG_PALLETTI_II, "PALLETTI-II HK2 TV2", u16(22));
  float hk1_tr = r_.temp_u16(TAG_PALLETTI_II, "PALLETTI-II HK1 TR", u16(24));
  float hk2_tr2 = r_.temp_u16(TAG_PALLETTI_II, "PALLETTI-II HK2 TR2", u16(26));
  float tpo = r_.temp_u16(TAG_PALLETTI_II, "PALLETTI-II TPO", u16(28));
  float tpu = r_.temp_u16(TAG_PALLETTI_II, "PALLETTI-II TPU", u16(30));
  float tzr = r_.temp_u16(TAG_PALLETTI_II, "PALLETTI-II TZR", u16(32));
  // Pump percentages, only present on the longer frame variants
  const bool has_pk = last_data >= 34;
  const bool has_phk1 = last_data >= 35;
  const bool has_phk2 = last_data >= 36;
  float pk = has_pk ? u8(34) / 1.0f : 0.0f;
  float hk1_phk = has_phk1 ? u8(35) / 1.0f : 0.0f;
  float hk2_phk2 = has_phk2 ? u8(36) / 1.0f : 0.0f;

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
  if (has_pk)
    r_.pub_palletti_ii_pk(pk);
  if (has_phk1)
    r_.pub_palletti_ii_hk1_phk(hk1_phk);
  if (has_phk2)
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
    // last_display_ bewusst NICHT aktualisieren: nach Ablauf wird der dann
    // aktuelle Text veröffentlicht, sonst bliebe der Sensor auf altem Text
    return;
  }

  ESP_LOGD(TAG_PALLETTI_II, "Display-Text geändert: \"%s\"", ascii.c_str());
  r_.pub_palletti_ii_display_text(ascii);
  last_display_   = ascii;
  last_display_ms_= now;
}

}  // namespace systa_reader
}  // namespace esphome

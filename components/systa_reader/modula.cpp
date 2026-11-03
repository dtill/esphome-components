#include "modula.h"
#include "esphome/core/log.h"

namespace esphome {
namespace systa_reader {

static const char *const TAG_MODULA = "systa_reader.modula";

void ModulaDecoder::on_fc_frame(const std::vector<uint8_t>& frame,
                                const std::vector<uint8_t>& payload,
                                const std::string & /*hex*/) {
  // FC .. 0C 01  (nur MODULA)
  if (frame.size() < 6 || frame[0] != 0xFC || frame[2] != 0x0C || frame[3] != 0x01) return;

  const uint8_t len = frame[1];
  // Wir lesen bis frame[33] (u16 bei 32..33) -> total = 2 + len + 1 muss > 33 -> len >= 32
  if (len < 32) {
    #if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_VERBOSE
    ESP_LOGW(TAG_MODULA, "Frame too short for MODULA: len=%u", len);
    #endif
    return;
  }

  // Zeitstempel aus payload (BCD): day, month, minute, hour
  if (payload.size() >= 4) {
    uint8_t day    = bcd2dec(payload[0]);
    uint8_t month  = bcd2dec(payload[1]);
    uint8_t minute = bcd2dec(payload[2]);
    uint8_t hour   = bcd2dec(payload[3]);
    char ts[16];
    snprintf(ts, sizeof(ts), "%02d.%02d %02d:%02d", day, month, hour, minute);
    r_.pub_modula_timestamp(ts);
  }

  // Werte (wie in deiner Vorlage) – Indizes sind **am Gesamtframe** (Big-Endian)
  auto u16 = [&](int i){ return read_u16_be(frame, i); };
  auto s16 = [&](int i){ return (int16_t)read_u16_be(frame, i); };

  float ta = r_.temp_s16(TAG_MODULA, "MODULA TA", s16(8));
  float two = r_.temp_u16(TAG_MODULA, "MODULA TWO", u16(10));
  float tbv = r_.temp_u16(TAG_MODULA, "MODULA TBV", u16(12));
  float tbr = r_.temp_u16(TAG_MODULA, "MODULA TBR", u16(14));
  float tv  = r_.temp_u16(TAG_MODULA, "MODULA TV", u16(20));
  float tv2 = r_.temp_u16(TAG_MODULA, "MODULA TV2", u16(22));
  float tr  = r_.temp_u16(TAG_MODULA, "MODULA TR", u16(24));
  float tr2 = r_.temp_u16(TAG_MODULA, "MODULA TR2", u16(26));
  float tpo = r_.temp_u16(TAG_MODULA, "MODULA TPO", u16(28));
  float tpu = r_.temp_u16(TAG_MODULA, "MODULA TPU", u16(30));
  float tzr = r_.temp_u16(TAG_MODULA, "MODULA TZR", u16(32));

  r_.pub_modula_ta(ta);
  r_.pub_modula_two(two);
  r_.pub_modula_tbv(tbv);
  r_.pub_modula_tbr(tbr);
  r_.pub_modula_tv(tv);
  r_.pub_modula_tv2(tv2);
  r_.pub_modula_tr(tr);
  r_.pub_modula_tr2(tr2);
  r_.pub_modula_tpo(tpo);
  r_.pub_modula_tpu(tpu);
  r_.pub_modula_tzr(tzr);

  ESP_LOGI(TAG_MODULA, "MODULA: TA=%.1f TWO=%.1f TV=%.1f TV2=%.1f TR=%.1f TR2=%.1f TPO=%.1f TPU=%.1f TZR=%.1f TBV=%.1f TBR=%.1f",
           ta, two, tv, tv2, tr, tr2, tpo, tpu, tzr, tbv, tbr);
}

} // namespace systa_reader
} // namespace esphome

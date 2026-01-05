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

  float ta = s16(8) / 10.0f;
  float two = u16(10) / 10.0f;
  float tbv = u16(12) / 10.0f;
  float tbr = u16(14) / 10.0f;
  float tv  = u16(20) / 10.0f;
  float tv2 = u16(22) / 10.0f;
  float tr  = u16(24) / 10.0f;
  float tr2 = u16(26) / 10.0f;
  float tpo = u16(28) / 10.0f;
  float tpu = u16(30) / 10.0f;
  float tzr = u16(32) / 10.0f;

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

#include "compact.h"
#include "esphome/core/log.h"

namespace esphome {
namespace systa_reader {

static const char *const TAG_ESP = "systa_reader.compact";

void CompactDecoder::on_fc_frame(const std::vector<uint8_t>& frame,
                                  const std::vector<uint8_t>& payload,
                                  const std::string &/*hex*/) {
  // Nur Compact-Frames: FC .. 0D 01 ..
  if (frame.size() < 6 || frame[0] != 0xFC || frame[2] != 0x0D || frame[3] != 0x01)
    return;

  // Letztes Pflicht-Register ist u16 bei frame[22..23]. Das letzte Datenbyte
  // liegt bei frame.size() - 2, danach folgt die Checksumme. Ohne diesen Guard
  // liest ein kurzer Frame über das Ende hinaus und liefert unsinnige Werte.
  const int last_data = (int) frame.size() - 2;
  if (last_data < 23) {
    ESP_LOGW(TAG_ESP, "Frame too short: len=%u, need at least 22", frame[1]);
    return;
  }

  auto u16 = [&](int i){ return read_u16_be(frame, i); };
  auto s16 = [&](int i){ return (int16_t)read_u16_be(frame, i); };
  auto u8  = [&](int i){ return read_u8(frame, i); };

  // Zeitstempel
  uint8_t day    = bcd2dec(payload[0]);
  uint8_t month  = bcd2dec(payload[1]);
  uint8_t minute = bcd2dec(payload[2]);
  uint8_t hour   = bcd2dec(payload[3]);
  char ts[16];
  snprintf(ts, sizeof(ts), "%02d.%02d %02d:%02d", day, month, hour, minute);
  r_.pub_compact_timestamp(ts);

  uint8_t status_raw = payload.size() > 20 ? payload[20] : 0; // 1 Byte
  uint16_t status_code = static_cast<uint16_t>(status_raw);

  // Werte gemäß Vorlage
  float ta = r_.temp_s16(TAG_ESP, "COMPACT TA", s16(8));
  float two = r_.temp_u16(TAG_ESP, "COMPACT TWO", u16(10));
  float fa_tv = r_.temp_u16(TAG_ESP, "COMPACT FA TV", u16(12));
  float fa_tr = r_.temp_u16(TAG_ESP, "COMPACT FA TR", u16(14));
  float ti = r_.temp_u16(TAG_ESP, "COMPACT TI", u16(16));
  float ti_s = u16(18) / 10.0f;
  float tv_s  = u16(20) / 10.0f;
  float two_s = u16(22) / 10.0f;

  r_.pub_compact_ta(ta);
  r_.pub_compact_two(two);
  r_.pub_compact_fa_tv(fa_tv);
  r_.pub_compact_fa_tr(fa_tr);
  r_.pub_compact_ti(ti);
  r_.pub_compact_ti_s(ti_s);
  r_.pub_compact_tv_s (tv_s);
  r_.pub_compact_two_s (two_s);
  r_.pub_compact_status_code(status_code);

  ESP_LOGI(TAG_ESP,
    "COMPACT: TA=%.1f TWO=%.1f FA TV=%.1f FA TR=%.1f TI=%.1f TI SOLL=%.1f TV SOLL=%.1f TWO SOLL=%.1f STATUS(dec)=%u STATUS(hex)=0x%02X",
    ta, two, fa_tv, fa_tr, ti, ti_s, tv_s, two_s,
    status_code, status_raw);
}

}  // namespace systa_reader
}  // namespace esphome

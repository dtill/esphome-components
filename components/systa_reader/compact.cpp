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

  auto u16 = [&](int i){ return read_u16_be(frame, i); };
  auto u8  = [&](int i){ return read_u8(frame, i); };

  // Zeitstempel
  uint8_t day    = bcd2dec(payload[0]);
  uint8_t month  = bcd2dec(payload[1]);
  uint8_t minute = bcd2dec(payload[2]);
  uint8_t hour   = bcd2dec(payload[3]);
  char ts[16];
  snprintf(ts, sizeof(ts), "%02d.%02d %02d:%02d", day, month, hour, minute);
  r_.pub_compact_timestamp(ts);

  // Werte gemäß Vorlage
  float ta = u16(8) / 10.0f;
  float two = u16(10) / 10.0f;
  float fa_tv = u16(12) / 10.0f;
  float fa_tr = u16(14) / 10.0f;
  float ti = u16(16) / 10.0f;
  float ti_s = u16(18) / 10.0f;
  float tv_s  = u16(20) / 10.0f;
  float two_s = u16(24) / 10.0f;
  float tzr = u16(26) / 10.0f;

  r_.pub_compact_ta(ta);
  r_.pub_compact_two(two);
  r_.pub_compact_fa_tv(fa_tv);
  r_.pub_compact_fa_tr(fa_tr);
  r_.pub_compact_ti(ti);
  r_.pub_compact_ti_s(ti_s);
  r_.pub_compact_tv_s (tv_s);
  r_.pub_compact_two_s (two_s);
  r_.pub_compact_tzr(tzr);

  ESP_LOGI(TAG_ESP, "COMPACT: TA=%.1f TWO=%.1f FA TV=%.1f FA TR=%.1f TI=%.1f TI SOLL=%.1f TV SOLL=%.1f TWO SOLL=%.1f TZR=%.1f",
           ta,two,fa_tv,fa_tr,ti,ti_s,tv_s,two_s,tzr);
}

}  // namespace systa_reader
}  // namespace esphome

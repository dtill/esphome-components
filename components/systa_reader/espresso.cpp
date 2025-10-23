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

  // Mindestlänge prüfen: wir lesen bis Index 34 (T12)
  if (payload.size() < 35) return;

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
  float t01 = u16(10) / 10.0f;
  float t02 = u16(12) / 10.0f;
  float t03 = u16(14) / 10.0f;
  float t04 = u16(16) / 10.0f;
  float t05 = u16(18) / 10.0f;
  float pk  = u16(20) / 1.0f;
  float t06 = u16(22) / 10.0f;
  float t07 = u16(24) / 10.0f;
  float t08 = u16(26) / 10.0f;
  float t09 = u16(28) / 10.0f;
  float t10 = u16(30) / 10.0f;
  float t11 = u16(32) / 10.0f;
  float t12 = u16(34) / 10.0f;

  r_.pub_espresso_t01(t01);
  r_.pub_espresso_t02(t02);
  r_.pub_espresso_t03(t03);
  r_.pub_espresso_t04(t04);
  r_.pub_espresso_t05(t05);
  r_.pub_espresso_pk (pk);
  r_.pub_espresso_t06(t06);
  r_.pub_espresso_t07(t07);
  r_.pub_espresso_t08(t08);
  r_.pub_espresso_t09(t09);
  r_.pub_espresso_t10(t10);
  r_.pub_espresso_t11(t11);
  r_.pub_espresso_t12(t12);

  ESP_LOGD(TAG_ESP, "ESP: %s | T01=%.1f T02=%.1f T03=%.1f T04=%.1f T05=%.1f PK=%.1f T06=%.1f T07=%.1f T08=%.1f T09=%.1f T10=%.1f T11=%.1f T12=%.1f",
           ts, t01,t02,t03,t04,t05,pk,t06,t07,t08,t09,t10,t11,t12);
}

}  // namespace systa_reader
}  // namespace esphome

#include "expresso_ii.h"
#include "esphome/core/log.h"

namespace esphome {
namespace systa_reader {

static const char *const TAG_EXP2 = "systa_reader.expresso_ii";

// payload[0..1] = minutes since midnight, payload[2..3] = days since
// 2000-01-01, both big-endian. Same encoding as the Comfort keypad's "UHR"
// command; verified across a day rollover.
void Expresso2Decoder::format_timestamp_(uint16_t mins, uint16_t days,
                                         char *out, size_t n) {
  int yy = 2000;
  while (true) {
    const bool leap = (yy % 4 == 0 && (yy % 100 != 0 || yy % 400 == 0));
    const int yd = leap ? 366 : 365;
    if ((int) days < yd)
      break;
    days -= yd;
    yy++;
  }
  int md[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (yy % 4 == 0 && (yy % 100 != 0 || yy % 400 == 0))
    md[1] = 29;
  int mo = 0;
  while (mo < 12 && (int) days >= md[mo]) {
    days -= md[mo];
    mo++;
  }
  snprintf(out, n, "%02u.%02u %02u:%02u", unsigned(days + 1), unsigned(mo + 1),
           unsigned(mins / 60), unsigned(mins % 60));
}

void Expresso2Decoder::on_fc_frame(const std::vector<uint8_t> &frame,
                                   const std::vector<uint8_t> &payload,
                                   const std::string &hex) {
  // EXPRESSO-II frames only: FC 37 14 01 ..
  if (frame.size() < 6 || frame[0] != 0xFC || frame[2] != 0x14 ||
      frame[3] != 0x01)
    return;

  r_.publish_hex_expresso_ii(hex);
  ESP_LOGV(TAG_EXP2, "EXPRESSO-II len=%u, frame.size()=%u, payload.size()=%u",
           frame[1], (unsigned) frame.size(), (unsigned) payload.size());

  if (payload.size() < 4)
    return;

  // Timestamp, payload[0..3] — see format_timestamp_
  char ts[16];
  format_timestamp_(read_u16_be(payload, 0), read_u16_be(payload, 2), ts,
                    sizeof(ts));
  r_.pub_expresso_ii_timestamp(ts);

  // ---- Identified fields
  // TA appears to be the damped outside temperature the controller uses for
  // its heating curve, not the raw sensor: it barely moves over hours.
  const float ta = read_s16_be(frame, 8) / 10.0f;   // outside temperature
  const float two = read_u16_be(frame, 10) / 10.0f; // domestic hot water
  // TKW drops to mains temperature while water is drawn and warms up again
  // when idle (stagnation in the cold water pipe).
  const float tkw = read_u16_be(frame, 12) / 10.0f;    // cold water [degC]
  const float dfl_tw = read_u16_be(frame, 14) / 10.0f; // DHW flow [l/min]
  const float tsp = read_u16_be(frame, 16) / 10.0f;    // storage tank [degC]
  const float two_s = read_u16_be(frame, 20) / 10.0f;  // DHW setpoint [degC]
  // Two measuring points of the same heating water circuit: the readings
  // track each other almost exactly (mass balance). Which one is flow and
  // which is return is not established, hence the neutral 1/2.
  const float dfl_hz1 = read_u16_be(frame, 22) / 10.0f; // [l/min]
  const float dfl_hz2 = read_u16_be(frame, 24) / 10.0f; // [l/min]
  const float tsp_s = read_u16_be(frame, 26) / 10.0f;  // tank setpoint [degC]
  const uint8_t pk = read_u8(frame, 34);   // boiler load [%]
  const uint8_t phk = read_u8(frame, 35);  // heating circuit pump
  // Storage pump: u8, not scaled, tops out at exactly 100. Tracks the
  // heating water flow closely — this is the pump feeding the heat
  // exchanger.
  const uint8_t p_sp = read_u8(frame, 41); // storage pump [%]

  r_.pub_expresso_ii_ta(ta);
  r_.pub_expresso_ii_two(two);
  r_.pub_expresso_ii_tkw(tkw);
  r_.pub_expresso_ii_dfl_tw(dfl_tw);
  r_.pub_expresso_ii_tsp(tsp);
  r_.pub_expresso_ii_two_s(two_s);
  r_.pub_expresso_ii_dfl_hz1(dfl_hz1);
  r_.pub_expresso_ii_dfl_hz2(dfl_hz2);
  r_.pub_expresso_ii_tsp_s(tsp_s);
  r_.pub_expresso_ii_pk(pk);
  r_.pub_expresso_ii_phk(phk);
  r_.pub_expresso_ii_p_sp(p_sp);

  // ---- Registers that are not identified yet
  // Published without a unit and without an interpretation, so nothing
  // misleading reaches the frontend. Once a register is confirmed it moves
  // up to the named values above. Offset 32 is the only signed one; read as
  // u16 it would show values around 6550 instead of small negatives.
  for (int off : {18, 28, 30, 36, 38, 42, 44, 46, 48})
    r_.pub_expresso_ii_raw(off, read_u16_be(frame, off) / 10.0f);
  r_.pub_expresso_ii_raw(32, read_s16_be(frame, 32) / 10.0f);

  ESP_LOGI(TAG_EXP2,
           "EXPRESSO-II: ZEIT=%s TA=%.1f TWO=%.1f TKW=%.1f TSP=%.1f "
           "TWO_S=%.1f TSP_S=%.1f DFL_TW=%.1f DFL_HZ1=%.1f DFL_HZ2=%.1f "
           "PK=%u PHK=%u P_SP=%u",
           ts, ta, two, tkw, tsp, two_s, tsp_s, dfl_tw, dfl_hz1, dfl_hz2, pk,
           phk, p_sp);
}

void Expresso2Decoder::on_fd_version_frame(uint8_t major, uint8_t minor,
                                           uint8_t patch) {
  char buf[16];
  snprintf(buf, sizeof(buf), "%u.%u.%u", major, minor, patch);
  ESP_LOGI(TAG_EXP2, "EXPRESSO-II: Firmware=%s", buf);
  r_.pub_expresso_ii_fw_version(buf);
}

} // namespace systa_reader
} // namespace esphome

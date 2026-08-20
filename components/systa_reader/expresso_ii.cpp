#include "expresso_ii.h"
#include "esphome/core/log.h"

namespace esphome {
namespace systa_reader {

static const char *const TAG_EXP2 = "systa_reader.expresso_ii";

// payload[0..1] = Minuten seit Mitternacht, payload[2..3] = Tage seit
// 2000-01-01 — beides Big-Endian, identisch zum "UHR"-Kommando des
// Comfort-Keypads. Bestätigt über den Tageswechsel 9727 -> 9728, der im
// Kundenlog exakt dem Wechsel 19.08. -> 20.08.2026 entspricht.
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
  // Nur EXPRESSO-II Frames: FC 37 14 01 ..
  if (frame.size() < 6 || frame[0] != 0xFC || frame[2] != 0x14 ||
      frame[3] != 0x01)
    return;

  r_.publish_hex_expresso_ii(hex);
  ESP_LOGV(TAG_EXP2, "EXPRESSO-II len=%u, frame.size()=%u, payload.size()=%u",
           frame[1], (unsigned) frame.size(), (unsigned) payload.size());

  if (payload.size() < 4)
    return;

  // Zeitstempel (payload[0..3], siehe format_timestamp_)
  char ts[16];
  format_timestamp_(read_u16_be(payload, 0), read_u16_be(payload, 2), ts,
                    sizeof(ts));
  r_.pub_expresso_ii_timestamp(ts);

  // ---- Gesicherte Felder
  // Gegen das Kundenlog vom 19./20.08.2026 verifiziert: TA folgt dem
  // Tagesgang, TWO kühlt über Nacht aus, PK steigt mit der Kesselleistung.
  const float ta = read_s16_be(frame, 8) / 10.0f;   // Außentemperatur
  const float two = read_u16_be(frame, 10) / 10.0f; // Warmwasser / Kessel
  const uint8_t pk = read_u8(frame, 34);            // Kesselleistung [%]
  const uint8_t phk = read_u8(frame, 35);           // Pumpe Heizkreis

  r_.pub_expresso_ii_ta(ta);
  r_.pub_expresso_ii_two(two);
  r_.pub_expresso_ii_pk(pk);
  r_.pub_expresso_ii_phk(phk);

  // ---- Noch nicht zugeordnete Register
  // Der Frame ist 58 Byte lang und trägt deutlich mehr Kanäle als Espresso I.
  // Bis zum Abgleich gegen das Reglerdisplay werden sie als BE-u16/10 unter
  // ihrem Frame-Offset veröffentlicht. Ist ein Feld gesichert, wandert es
  // nach oben zu den benannten Werten.
  for (int off = SystaReader::kExpresso2RawFirst;
       off <= SystaReader::kExpresso2RawLast; off += 2) {
    if (off == 34) // 34/35 sind PK/PHK (zwei u8), kein u16
      continue;
    r_.pub_expresso_ii_raw(off, read_u16_be(frame, off) / 10.0f);
  }

  ESP_LOGI(TAG_EXP2, "EXPRESSO-II: ZEIT=%s TA=%.1f TWO=%.1f PK=%u PHK=%u", ts,
           ta, two, pk, phk);
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

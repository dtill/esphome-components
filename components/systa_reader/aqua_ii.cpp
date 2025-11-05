#include "aqua_ii.h"
#include "esphome/core/log.h"

namespace esphome {
namespace systa_reader {

static const char *const TAG_AQUA_II = "systa_reader.aqua_ii";

const char *Aqua2Decoder::status_text(uint8_t raw) {
  switch (raw) {
    case 0:  return "Warten auf Sonne";
    case 1:  return "Frostschutz";
    case 2:  return "Anschieben";
    case 3:  return "Einschaltverzögerung";
    case 4:  return "Einspeisen";
    case 5:  return "Speicher voll";
    case 6:  return "Kollektor überhitzt";
    case 7:  return "Manuell (TEST, HAND, AUS)";
    case 8:  return "Messung";
    case 9:  return "Störabschaltung";
    default: return nullptr;
  }
}
const char *Aqua2Decoder::status_stoer_text(uint8_t raw) {
  switch (raw) {
    case 0:  return "Kein Fehler";
    case 1:  return "Durchfluss im Solarkreis blockiert oder Pumpe defekt";
    case 2:  return "Luft in der Anlage";
    case 3:  return "Kein Volumenstrom im Frostschutz";
    case 4:  return "Vorlauf-/Rücklauf Kollektor vertauscht";
    case 5:  return "Rückschlagklappe undicht";
    case 6:  return "Falsche Uhrzeit";
    case 7:  return "Druckabfall in der Anlage";
    case 8:  return "Volumenstrom zu hoch";
    case 9:  return "Hydraulischer Anschluss fehlerhaft";
    case 10: return "Anlage nicht frostsicher";
    case 11: return "Keine permanente Spannungsversorgung";
    case 12: return "Speicherfühler/ULV/Wärmetauscher Problem";
    case 13: return "Volumenstrom zu niedrig";
    case 14: return "Speicher unterkühlt";
    case 22: return "Fühler TSA defekt";
    case 23: return "Fühler TSE defekt";
    case 24: return "Fühler TWU defekt";
    case 26: return "Fühler TW2 defekt";
    case 34: return "Speicher überhitzt";
    case 35: return "Speicher 2 überhitzt";
    case 50: return "Frostgefahr";
    default: return nullptr;
  }
}

void Aqua2Decoder::on_fc_frame(const std::vector<uint8_t> &frame,
                               const std::vector<uint8_t> &payload,
                               const std::string &hex) {
  // Nur FC 3E 24 01
  if (frame.size() < 6 || frame[0] != 0xFC || frame[2] != 0x24 || frame[3] != 0x01)
    return;

  r_.publish_hex_aqua_ii(hex);

  ESP_LOGV(TAG_AQUA_II, "AQUA-II len=%u, frame.size()=%u, payload.size()=%u",
           frame[1], (unsigned)frame.size(), (unsigned)payload.size());

  // ---- Werte gemäß decode-systa-csv.sh
  float tsa  = read_i16_le(payload,  0) / 10.0f; // TSA1 (Kollektor)
  float tw   = read_i16_le(payload,  2) / 10.0f; // TW Speicher
  float tsv  = read_i16_le(payload,  4) / 10.0f; // TSV Vorlauf
  float tam  = read_i16_le(payload,  6) / 10.0f; // TAM Außen
  float tse  = read_i16_le(payload, 10) / 10.0f; // TSE Rücklauf
  float dfl  = read_i16_le(payload, 12) / 10.0f; // Durchfluss (0.1 l/min)
  uint8_t pwm    = payload.size() > 14 ? payload[14] : 0; // PWM Pumpe (%)
  uint8_t status = payload.size() > 19 ? payload[19] : 0; // Status (Byte)

  // Zeit/Datum: KEIN BCD (gemäß deinem Skript)
  uint8_t h  = payload.size() > 22 ? payload[22] : 0; // Stunde
  uint8_t m  = payload.size() > 23 ? payload[23] : 0; // Minute
  uint8_t d  = payload.size() > 24 ? payload[24] : 0; // Tag
  uint8_t mo = payload.size() > 25 ? payload[25] : 0; // Monat
  uint8_t y  = payload.size() > 26 ? payload[26] : 0; // Jahr (00..99)

  uint16_t koll_lstg = read_u16_le(payload, 29);       // Kollektorleistung
  uint16_t tag_erg = read_u16_le(payload, 29);       // Tagesleistung
  uint32_t gesamt  = read_u32_le(payload, 33);       // Gesamtleistung

  // ---- Publish
  r_.pub_aqua_ii_tsa(tsa);
  r_.pub_aqua_ii_twu(tw);
  r_.pub_aqua_ii_tsv(tsv);
  r_.pub_aqua_ii_tam(tam);
  r_.pub_aqua_ii_tse(tse);
  r_.pub_aqua_ii_dfl(dfl);
  r_.pub_aqua_ii_pwm(pwm);
  r_.pub_aqua_ii_koll_lstg(koll_lstg);
  r_.pub_aqua_ii_tag(tag_erg);
  r_.pub_aqua_ii_ges(gesamt);
  r_.pub_aqua_ii_status_code(status);

  if (const char *t = status_text(status)) {
    r_.pub_aqua_ii_status_text(t);
  } else {
    char buf[40];
    snprintf(buf, sizeof(buf), "Unbekannter Status (%02X)", status);
    r_.pub_aqua_ii_status_text(buf);
  }

  char ts[20];
  snprintf(ts, sizeof(ts), "%02u.%02u.%02u %02u:%02u", d, mo, y, h, m);
  r_.pub_aqua_ii_timestamp(ts);

  ESP_LOGI(TAG_AQUA_II,
           "AQUA-II: TSA=%.1f TW=%.1f TSV=%.1f TAM=%.1f TSE=%.1f DFL=%.1f PWM=%u LSTG=%u TAG=%u GES=%u Status=%02X @ %02u.%02u.%02u %02u:%02u",
           tsa, tw, tsv, tam, tse, dfl, pwm, koll_lstg, tag_erg, gesamt, status, d, mo, y, h, m);
}

}  // namespace systa_reader
}  // namespace esphome

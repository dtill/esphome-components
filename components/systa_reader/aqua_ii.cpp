#include "aqua_ii.h"
#include "esphome/core/log.h"

namespace esphome {
namespace systa_reader {

static const char *const TAG_AQUA_II = "systa_reader.aqua_ii";

const char *Aqua2Decoder::status_text(uint8_t raw) {
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
  // FC 3E 24 01
  if (frame.size() < 6 || frame[0] != 0xFC || frame[2] != 0x24 || frame[3] != 0x01)
    return;

  r_.publish_hex_aqua_ii(hex);

  ESP_LOGV(TAG_AQUA_II, "AQUA-II len=%u, frame.size()=%u, payload.size()=%u",
           frame[1], (unsigned)frame.size(), (unsigned)payload.size());

  // Werte (LE)
  float tsa = Aqua2Decoder::read_i16_le(payload, 2)  / 10.0f;
  float tw  = Aqua2Decoder::read_i16_le(payload, 4)  / 10.0f;
  float tsv = Aqua2Decoder::read_i16_le(payload, 6)  / 10.0f;
  float tam = Aqua2Decoder::read_i16_le(payload, 8)  / 10.0f;
  float tse = Aqua2Decoder::read_i16_le(payload,12)  / 10.0f;
  float dfl = Aqua2Decoder::read_i16_le(payload,14)  / 10.0f;
  uint8_t pwm    = payload.size() > 16 ? payload[16] : 0;
  uint8_t status = payload.size() > 21 ? payload[21] : 0;

  // Zeit/Datum (BCD-kodiert! Reihenfolge: Tag, Monat, Minute, Stunde, Jahr)
  uint8_t d_raw  = payload.size() > 24 ? payload[24] : 0;
  uint8_t mo_raw = payload.size() > 25 ? payload[25] : 0;
  uint8_t m_raw  = payload.size() > 26 ? payload[26] : 0;
  uint8_t h_raw  = payload.size() > 27 ? payload[27] : 0;
  uint8_t y_raw  = payload.size() > 28 ? payload[28] : 0;

  ESP_LOGV(TAG_AQUA_II, "Time BCD raw: D=%02X MO=%02X M=%02X H=%02X Y=%02X",
         d_raw, mo_raw, m_raw, h_raw, y_raw);

  // BCD → Dezimal
  uint8_t d  = bcd2dec(d_raw);
  uint8_t mo = bcd2dec(mo_raw);
  uint8_t m  = bcd2dec(m_raw);
  uint8_t h  = bcd2dec(h_raw);
  uint8_t y  = bcd2dec(y_raw);

  uint16_t tag_erg = Aqua2Decoder::read_u16_le(payload, 31);
  uint32_t gesamt  = Aqua2Decoder::read_u32_le(payload, 35);

  // Debug (optional)
  ESP_LOGD(TAG_AQUA_II, "P[2]=%04X P[4]=%04X P[6]=%04X P[8]=%04X P[12]=%04X P[14]=%04X",
           Aqua2Decoder::read_u16_le(payload,2), Aqua2Decoder::read_u16_le(payload,4),
           Aqua2Decoder::read_u16_le(payload,6), Aqua2Decoder::read_u16_le(payload,8),
           Aqua2Decoder::read_u16_le(payload,12), Aqua2Decoder::read_u16_le(payload,14));

  // Publish numerisch
  r_.pub_aqua_ii_tsa(tsa);
  r_.pub_aqua_ii_twu(tw);
  r_.pub_aqua_ii_tsv(tsv);
  r_.pub_aqua_ii_tam(tam);
  r_.pub_aqua_ii_tse(tse);
  r_.pub_aqua_ii_dfl(dfl);
  r_.pub_aqua_ii_pwm(pwm);
  r_.pub_aqua_ii_tag(tag_erg);
  r_.pub_aqua_ii_ges(gesamt);
  r_.pub_aqua_ii_status_code(status);

  // Publish Textsensoren (AQUA-II-**Slots** weiterverwenden)
  if (const char *t = status_text(status)) {
    r_.pub_aqua_ii_status_text(t);
  } else {
    char buf[40];
    snprintf(buf, sizeof(buf), "Unbekannter Status (%02X)", status);
    r_.pub_aqua_ii_status_text(buf);
  }

  // Format "DD.MM.YY HH:MM"
  char ts[20];
  snprintf(ts, sizeof(ts), "%02u.%02u.%02u %02u:%02u", d, mo, y, h, m);
  r_.pub_aqua_ii_timestamp(ts);

  ESP_LOGI(TAG_AQUA_II,
           "AQUA-II: TSA=%.1f TW=%.1f TSV=%.1f TAM=%.1f TSE=%.1f DFL=%.1f PWM=%u TAG=%u GES=%u Status=%02X",
           tsa, tw, tsv, tam, tse, dfl, pwm, tag_erg, gesamt, status);
}

}  // namespace systa_reader
}  // namespace esphome

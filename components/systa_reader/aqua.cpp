#include "aqua.h"
#include "esphome/core/log.h"

namespace esphome {
namespace systa_reader {

static const char *const TAG_AQUA = "systa_reader.aqua";

const char* AquaDecoder::status_text(uint8_t raw) {
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

void AquaDecoder::on_fc_frame(const std::vector<uint8_t>& frame,
                              const std::vector<uint8_t>& payload,
                              const std::string &hex) {
  // Nur AQUA Frames: FC .. 0B 01 ..
  if (frame.size()<6 || frame[0]!=0xFC || frame[2]!=0x0B || frame[3]!=0x01) return;

  // HEX auch in AQUA-only sink
  r_.publish_hex_aqua(hex);
  ESP_LOGV(TAG_AQUA, "AQUA len=%u, frame.size()=%u, payload.size()=%u",
         frame[1], (unsigned)frame.size(), (unsigned)payload.size());
  //if (payload.size() < 18) return;

  auto u16 = [&](int i){ return read_u16_be(frame, i); };
  auto u32 = [&](int i){ return read_u32_be(frame, i); };

  float tsa    = u16(4)  / 10.0f;
  float tse    = u16(6)  / 10.0f;
  float twu    = u16(8)  / 10.0f;
  float tw2    = u16(10) / 10.0f;
  float sol    = u16(24);
  float tag    = u16(26);
  float gesamt = u32(28);

  uint8_t status_raw  = payload[11];
  uint8_t status_code = uint8_t((status_raw/10) * 16 + (status_raw%10));

  r_.pub_aqua_tsa(tsa);
  r_.pub_aqua_tse(tse);
  r_.pub_aqua_twu(twu);
  r_.pub_aqua_tw2(tw2);
  r_.pub_aqua_sol(sol);
  r_.pub_aqua_tag(tag);
  r_.pub_aqua_ges(gesamt);
  r_.pub_aqua_status_code(status_code);

  if (const char* t = status_text(status_raw)) {
    r_.pub_aqua_status_text(t);
  } else {
    char buf[40];
    snprintf(buf, sizeof(buf), "Unbekannter Status (%02X)", status_code);
    r_.pub_aqua_status_text(buf);
  }

  uint8_t hour   = bcd2dec(payload[14]);
  uint8_t minute = bcd2dec(payload[15]);
  uint8_t day    = bcd2dec(payload[16]);
  uint8_t month  = bcd2dec(payload[17]);
  char ts[16];
  snprintf(ts, sizeof(ts), "%02d.%02d %02d:%02d", day, month, hour, minute);
  r_.pub_aqua_timestamp(ts);

  ESP_LOGD(TAG_AQUA, "AQUA: TSA=%.1f TSE=%.1f TWU=%.1f TW2=%.1f SOL=%.0f TAG=%.0f GES=%.0f code=%02X",
           tsa, tse, twu, tw2, sol, tag, gesamt, status_code);
}
void AquaDecoder::on_display_frame(const std::vector<uint8_t>& /*frame*/,
                                   const std::vector<uint8_t>& payload,
                                   const std::string &/*hex*/) {
  // payload ist 32 Byte ASCII (mit evtl. non-printables → '.' ersetzen)
  std::string ascii; ascii.reserve(payload.size());
  for (auto b : payload) ascii += (b >= 32 && b <= 126) ? char(b) : '.';

  if (ascii == last_display_) return;  // unverändert

  const uint32_t now = millis();
  if (last_display_ms_ != 0 && (uint32_t)(now - last_display_ms_) < DISPLAY_MIN_INTERVAL_MS) {
    // innerhalb 5 min: nur leise loggen
    ESP_LOGV(TAG_AQUA, "Display change suppressed (interval): \"%s\"", ascii.c_str());
    last_display_ = ascii;  // trotzdem aktualisieren, damit nach Ablauf nicht „altes“ kommt
    return;
  }

  ESP_LOGD(TAG_AQUA, "Display-Text geändert: \"%s\"", ascii.c_str());
  r_.pub_aqua_display_text(ascii);
  last_display_   = ascii;
  last_display_ms_= now;
}

} // namespace systa_reader
} // namespace esphome

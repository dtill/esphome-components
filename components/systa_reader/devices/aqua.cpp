#include "aqua.h"
#include "esphome/core/log.h"

namespace esphome {
namespace systa_reader {

static const char *const TAG_AQUA = "systa_reader.aqua";

void AquaDevice::on_fc_frame(const std::vector<uint8_t>& frame,
                             const std::vector<uint8_t>& payload,
                             const std::string &hex) {
  // Nur FC .. 0B 01 .. verarbeiten
  if (frame.size()<4 || frame[0]!=0xFC || frame[2]!=0x0B || frame[3]!=0x01) return;

  // AQUA-HEX separat publishen
  for (auto *s : r_.sinks_aqua_) s->publish_frame_hex(hex);

  if (payload.size() < 30) return;

  auto read_u16 = [&](int i){ return DeviceBase::read_u16_be(frame, i); };
  auto read_u32 = [&](int i){ return DeviceBase::read_u32_be(frame, i); };

  float tsa    = read_u16(4)  / 10.0f;
  float tse    = read_u16(6)  / 10.0f;
  float twu    = read_u16(8)  / 10.0f;
  float tw2    = read_u16(10) / 10.0f;
  float sol    = read_u16(24);
  float tag    = read_u16(26);
  float gesamt = read_u32(28);

  uint8_t status_raw  = payload[11];
  uint8_t status_code = uint8_t((status_raw/10) * 16 + (status_raw%10));

  const char *desc = nullptr;
  switch (status_raw) {
    case 0:  desc="Kein Fehler"; break;
    case 1:  desc="Durchfluss im Solarkreis blockiert oder Pumpe defekt"; break;
    case 2:  desc="Luft in der Anlage"; break;
    case 3:  desc="Kein Volumenstrom im Frostschutz"; break;
    case 4:  desc="Vorlauf-/Rücklauf Kollektor vertauscht"; break;
    case 5:  desc="Rückschlagklappe undicht"; break;
    case 6:  desc="Falsche Uhrzeit"; break;
    case 7:  desc="Druckabfall in der Anlage"; break;
    case 8:  desc="Volumenstrom zu hoch"; break;
    case 9:  desc="Hydraulischer Anschluss fehlerhaft"; break;
    case 10: desc="Anlage nicht frostsicher"; break;
    case 11: desc="Keine permanente Spannungsversorgung"; break;
    case 12: desc="Speicherfühler/ULV/Wärmetauscher Problem"; break;
    case 13: desc="Volumenstrom zu niedrig"; break;
    case 14: desc="Speicher unterkühlt"; break;
    case 22: desc="Fühler TSA defekt"; break;
    case 23: desc="Fühler TSE defekt"; break;
    case 24: desc="Fühler TWU defekt"; break;
    case 26: desc="Fühler TW2 defekt"; break;
    case 34: desc="Speicher überhitzt"; break;
    case 35: desc="Speicher 2 überhitzt"; break;
    case 50: desc="Frostgefahr"; break;
    default: break;
  }

  uint8_t hour   = DeviceBase::bcd2dec(payload[14]);
  uint8_t minute = DeviceBase::bcd2dec(payload[15]);
  uint8_t day    = DeviceBase::bcd2dec(payload[16]);
  uint8_t month  = DeviceBase::bcd2dec(payload[17]);
  char ts[16]; snprintf(ts, sizeof(ts), "%02d.%02d %02d:%02d", day, month, hour, minute);

  r_.publish_numeric(Kind::AQUA_TSA,    tsa);
  r_.publish_numeric(Kind::AQUA_TSE,    tse);
  r_.publish_numeric(Kind::AQUA_TWU,    twu);
  r_.publish_numeric(Kind::AQUA_TW2,    tw2);
  r_.publish_numeric(Kind::AQUA_SOL,    sol);
  r_.publish_numeric(Kind::AQUA_TAG,    tag);
  r_.publish_numeric(Kind::AQUA_GESAMT, gesamt);
  r_.publish_numeric(Kind::AQUA_STATUS_CODE, status_code);

  if (desc) r_.publish_text(Kind::AQUA_STATUS_TEXT, desc);
  else {
    char buf[32];
    snprintf(buf, sizeof(buf), "Unbekannter Status (%02X)", status_code);
    r_.publish_text(Kind::AQUA_STATUS_TEXT, buf);
  }
  r_.publish_text(Kind::AQUA_TIMESTAMP, ts);

  ESP_LOGV(TAG_AQUA, "AQUA: TSA=%.1f TSE=%.1f TWU=%.1f TW2=%.1f SOL=%.0f TAG=%.0f GES=%.0f",
           tsa, tse, twu, tw2, sol, tag, gesamt);
}

}  // namespace systa_reader
}  // namespace esphome

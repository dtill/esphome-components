#include "systa_reader.h"
#include "esphome/core/log.h"

namespace esphome {
namespace systa_reader {

static const char *const TAG = "systa_reader";

void SystaReader::loop() {
  uint8_t b;
  while (this->available()) {
    if (!this->read_byte(&b)) break;
    buf_.push_back(b);
  }
  if (!buf_.empty()) process_buffer_();
}

void SystaReader::process_buffer_() {
  while (true) {
    if (buf_.size() < 4) return;

    // sync auf 0xFC/0x0F
    while (!buf_.empty() && buf_.front()!=0xFC && buf_.front()!=0x0F) buf_.pop_front();
    if (buf_.size() < 4) return;

    bool progressed = false;
    if (buf_.front()==0x0F)      progressed = try_parse_display_frame_();
    else if (buf_.front()==0xFC) progressed = try_parse_fc_frame_();

    if (!progressed) buf_.pop_front();
  }
}

bool SystaReader::try_parse_display_frame_() {
  if (buf_.size() < 4) return false;
  if (!(buf_[0]==0x0F && buf_[1]==0x22 && buf_[2]==0x04 && buf_[3]==0x00)) return false;

  const size_t total = 37;
  if (buf_.size() < total) return false;

  std::vector<uint8_t> frame(total);
  for (size_t i=0;i<total;i++) frame[i]=buf_[i];

  const uint8_t calc = checksum_twos_complement_(std::vector<uint8_t>(frame.begin(), frame.end()-1));
  const uint8_t got  = frame.back();
  if (calc != got && log_invalid_) ESP_LOGW(TAG, "Display checksum invalid (got %02X, expected %02X)", got, calc);

  const std::string hex = to_hex_(frame);
  for (auto *s : sinks_all_) s->publish_frame_hex(hex);
  ESP_LOGV(TAG, "Display HEX: %s", hex.c_str());

  for (size_t i=0;i<total;i++) buf_.pop_front();
  return true;
}

bool SystaReader::try_parse_fc_frame_() {
  if (buf_.size() < 3) return false;

  const uint8_t len = buf_[1];
  const size_t total = size_t(2) + len + 1;
  if (len < 2) { if (log_invalid_) ESP_LOGV(TAG, "Reject FC len=%u", len); return false; }
  if (buf_.size() < total) return false;

  std::vector<uint8_t> frame(total);
  for (size_t i=0;i<total;i++) frame[i]=buf_[i];

  const uint8_t calc = checksum_twos_complement_(std::vector<uint8_t>(frame.begin(), frame.end()-1));
  const uint8_t got  = frame.back();
  if (calc != got && log_invalid_) ESP_LOGW(TAG, "FC checksum invalid (got %02X, expected %02X)", got, calc);

  const std::string hex = to_hex_(frame);
  for (auto *s : sinks_all_) s->publish_frame_hex(hex);
  ESP_LOGV(TAG, "FC HEX: %s", hex.c_str());

  // AQUA-Frames: FC [len] 0B 01 ...
  if (device_type_ == "aqua" && frame.size() >= 6 && frame[0]==0xFC && frame[2]==0x0B && frame[3]==0x01) {
    // payload (nur Daten)
    std::vector<uint8_t> payload(frame.begin()+4, frame.end()-1);

    // Werte aus dem FULL FRAME (Big Endian)
    auto u16 = [&](int i){ return read_u16_be(frame, i); };
    auto u32 = [&](int i){ return read_u32_be(frame, i); };

    if (payload.size() >= 30) {
      float tsa    = u16(4)  / 10.0f;
      float tse    = u16(6)  / 10.0f;
      float twu    = u16(8)  / 10.0f;
      float tw2    = u16(10) / 10.0f;
      float sol    = u16(24);
      float tag    = u16(26);
      float gesamt = u32(28);

      uint8_t status_raw  = payload[11];
      uint8_t status_code = uint8_t((status_raw/10) * 16 + (status_raw%10));

      publish_numeric(Kind::AQUA_TSA, tsa);
      publish_numeric(Kind::AQUA_TSE, tse);
      publish_numeric(Kind::AQUA_TWU, twu);
      publish_numeric(Kind::AQUA_TW2, tw2);
      publish_numeric(Kind::AQUA_SOL, sol);
      publish_numeric(Kind::AQUA_TAG, tag);
      publish_numeric(Kind::AQUA_GESAMT, gesamt);
      publish_numeric(Kind::AQUA_STATUS_CODE, status_code);

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

      if (desc) publish_text(Kind::AQUA_STATUS_TEXT, desc);
      else {
        char buf[32];
        snprintf(buf, sizeof(buf), "Unbekannter Status (%02X)", status_code);
        publish_text(Kind::AQUA_STATUS_TEXT, buf);
      }

      uint8_t hour   = bcd2dec(payload[14]);
      uint8_t minute = bcd2dec(payload[15]);
      uint8_t day    = bcd2dec(payload[16]);
      uint8_t month  = bcd2dec(payload[17]);
      char ts[16]; snprintf(ts, sizeof(ts), "%02d.%02d %02d:%02d", day, month, hour, minute);
      publish_text(Kind::AQUA_TIMESTAMP, ts);
    }

    // AQUA-HEX nur für AQUA-Sink
    for (auto *s : sinks_aqua_) s->publish_frame_hex(hex);
  }

  for (size_t i=0;i<total;i++) buf_.pop_front();
  return true;
}

uint8_t SystaReader::checksum_twos_complement_(const std::vector<uint8_t> &v) {
  uint32_t sum=0; for (auto b: v) sum+=b;
  return static_cast<uint8_t>(0 - static_cast<int>(sum & 0xFF));
}

std::string SystaReader::to_hex_(const std::vector<uint8_t> &buf) {
  static const char *digits="0123456789ABCDEF";
  std::string out; out.reserve(buf.size()*2);
  for (au

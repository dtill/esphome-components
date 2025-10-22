#include "systa_reader.h"
#include "esphome/core/log.h"

namespace esphome {
namespace systa_reader {

static const char *const TAG = "systa_reader";

void SystaReader::loop() {
  uint8_t byte;
  while (this->available()) {
    if (this->read_byte(&byte)) buf_.push_back(byte);
    else break;
  }
  if (!buf_.empty()) this->process_buffer_();
}

void SystaReader::process_buffer_() {
  while (true) {
    if (buf_.size() < 4) return;

    while (!buf_.empty() && buf_.front() != 0xFC && buf_.front() != 0x0F) buf_.pop_front();
    if (buf_.size() < 4) return;

    bool progressed = false;
    if (buf_.front() == 0x0F)      progressed = this->try_parse_display_frame_();
    else if (buf_.front() == 0xFC) progressed = this->try_parse_fc_frame_();

    if (!progressed) buf_.pop_front();
  }
}

bool SystaReader::try_parse_display_frame_() {
  if (buf_.size() < 4) return false;
  if (!(buf_[0] == 0x0F && buf_[1] == 0x22 && buf_[2] == 0x04 && buf_[3] == 0x00)) return false;

  const size_t total = 37;
  if (buf_.size() < total) return false;

  std::vector<uint8_t> frame(total);
  for (size_t i = 0; i < total; i++) frame[i] = buf_[i];

  uint8_t calc = checksum_twos_complement_(std::vector<uint8_t>(frame.begin(), frame.end() - 1));
  uint8_t got  = frame.back();
  if (calc != got && this->log_invalid_) {
    ESP_LOGW(TAG, "Display frame checksum invalid (got %02X, expected %02X)", got, calc);
  }

  const std::string hex = to_hex_(frame);
  for (auto *s : sinks_all_) s->publish_frame_hex(hex);
  ESP_LOGV(TAG, "Display frame HEX: %s", hex.c_str());

  for (size_t i = 0; i < total; i++) buf_.pop_front();
  return true;
}

bool SystaReader::try_parse_fc_frame_() {
  if (buf_.size() < 3) return false;

  const uint8_t len = buf_[1];
  const size_t total = size_t(2) + len + 1;  // 0xFC, len, payload, checksum
  if (len < 2) { if (this->log_invalid_) ESP_LOGV(TAG, "Reject FC len=%u", len); return false; }
  if (buf_.size() < total) return false;

  std::vector<uint8_t> frame(total);
  for (size_t i = 0; i < total; i++) frame[i] = buf_[i];

  uint8_t calc = checksum_twos_complement_(std::vector<uint8_t>(frame.begin(), frame.end() - 1));
  uint8_t got  = frame.back();
  if (calc != got && this->log_invalid_) {
    ESP_LOGW(TAG, "FC checksum invalid (got %02X, expected %02X)", got, calc);
  }

  const std::string hex = to_hex_(frame);
  for (auto *s : sinks_all_) s->publish_frame_hex(hex);
  ESP_LOGV(TAG, "FC frame HEX: %s", hex.c_str());

  // AQUA?  FC [len] 0B 01 ...
  if (this->device_type_ == "aqua" && frame.size() >= 4 && frame[0] == 0xFC && frame[2] == 0x0B && frame[3] == 0x01) {
    std::vector<uint8_t> payload(frame.begin() + 4, frame.end() - 1);
    this->handle_aqua_payload_(frame, payload);
    // nur AQUA-HEX an spezielle Sinks
    for (auto *s : sinks_aqua_) s->publish_frame_hex(hex);
  }

  for (size_t i = 0; i < total; i++) buf_.pop_front();
  return true;
}

void SystaReader::handle_aqua_payload_(const std::vector<uint8_t> &frame, const std::vector<uint8_t> &payload) {
  if (payload.size() < 30) return;

  auto read_u16 = [&](int i) -> uint16_t { return read_u16_be_(frame, i); };
  auto read_u32 = [&](int i) -> uint32_t { return read_u32_be_(frame, i); };

  float tsa    = read_u16(4)  / 10.0f;
  float tse    = read_u16(6)  / 10.0f;
  float twu    = read_u16(8)  / 10.0f;
  float tw2    = read_u16(10) / 10.0f;
  float sol    = read_u16(24);      // kW
  float tag    = read_u16(26);      // kWh
  float gesamt = read_u32(28);      // kWh

  uint8_t status_raw  = payload[11];
  uint8_t status_code = uint8_t((status_raw / 10) * 16 + (status_raw % 10));
  const char *desc = nullptr;
  switch (status_raw) {
    case 0:  desc = "Kein Fehler"; break;
    case 1:  desc = "Durchfluss im Solarkreis blockiert oder Pumpe defekt"; break;
    case 2:  desc = "Luft in der Anlage"; break;
    case 3:  desc = "Kein Volumenstrom im Frostschutz"; break;
    case 4:  desc = "Vorlauf- und Rücklauf des Kollektors vertauscht"; break;
    case 5:  desc = "Rückschlagklappe undicht (Fehlzirkulation gegen Pumprichtung)"; break;
    case 6:  desc = "Falsche Uhrzeit"; break;
    case 7:  desc = "Druckabfall in der Anlage"; break;
    case 8:  desc = "Volumenstrom zu hoch"; break;
    case 9:  desc = "Hydraulischer Anschluss fehlerhaft (Fehlzirkulation in Pumprichtung)"; break;
    case 10: desc = "Anlage nicht frostsicher"; break;
    case 11: desc = "Keine permanente Spannungsversorgung"; break;
    case 12: desc = "Speicherfühler falsch gesetzt, ULV defekt oder Wärmetauscher verkalkt"; break;
    case 13: desc = "Volumenstrom zu niedrig"; break;
    case 14: desc = "Speicher unterkühlt"; break;
    case 22: desc = "Fühler TSA defekt"; break;
    case 23: desc = "Fühler TSE defekt"; break;
    case 24: desc = "Fühler TWU defekt"; break;
    case 26: desc = "Fühler TW2 defekt"; break;
    case 34: desc = "Speicher überhitzt"; break;
    case 35: desc = "Speicher 2 überhitzt"; break;
    case 50: desc = "Frostgefahr"; break;
    default: desc = nullptr; break;
  }

  uint8_t hour   = bcd2dec_(payload[14]);
  uint8_t minute = bcd2dec_(payload[15]);
  uint8_t day    = bcd2dec_(payload[16]);
  uint8_t month  = bcd2dec_(payload[17]);
  char timestamp[16];
  snprintf(timestamp, sizeof(timestamp), "%02d.%02d %02d:%02d", day, month, hour, minute);

  if (aqua_tsa_)          aqua_tsa_->publish_state(tsa);
  if (aqua_tse_)          aqua_tse_->publish_state(tse);
  if (aqua_twu_)          aqua_twu_->publish_state(twu);
  if (aqua_tw2_)          aqua_tw2_->publish_state(tw2);
  if (aqua_sol_)          aqua_sol_->publish_state(sol);
  if (aqua_tag_)          aqua_tag_->publish_state(tag);
  if (aqua_ges_)          aqua_ges_->publish_state(gesamt);
  if (aqua_status_code_)  aqua_status_code_->publish_state(status_code);
  if (aqua_status_text_) {
    if (desc) aqua_status_text_->publish_state(desc);
    else {
      char buf[32];
      snprintf(buf, sizeof(buf), "Unbekannter Status (%02X)", status_code);
      aqua_status_text_->publish_state(buf);
    }
  }
  if (aqua_timestamp_)    aqua_timestamp_->publish_state(timestamp);

  ESP_LOGV(TAG, "AQUA: TSA=%.1f TSE=%.1f TWU=%.1f TW2=%.1f SOL=%.0f TAG=%.0f GES=%.0f",
           tsa, tse, twu, tw2, sol, tag, gesamt);
}

uint8_t SystaReader::checksum_twos_complement_(const std::vector<uint8_t> &data_wo) {
  uint32_t sum = 0; for (auto b : data_wo) sum += b;
  return static_cast<uint8_t>(0 - static_cast<int>(sum & 0xFF));
}

std::string SystaReader::to_hex_(const std::vector<uint8_t> &buf) {
  static const char *digits = "0123456789ABCDEF";
  std::string out; out.reserve(buf.size() * 2);
  for (auto b : buf) { out.push_back(digits[(b>>4)&0xF]); out.push_back(digits[b&0xF]); }
  return out;
}

}  // namespace systa_reader
}  // namespace esphome

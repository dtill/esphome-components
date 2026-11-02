#pragma once
#include "systa_reader.h"
#include <vector>
#include <string>

namespace esphome {
namespace systa_reader {

class AquaDecoder {
 public:
  explicit AquaDecoder(SystaReader &owner) : r_(owner) {}
  void on_fc_frame(const std::vector<uint8_t>& frame,
                   const std::vector<uint8_t>& payload,
                   const std::string &hex);
  void on_display_frame(const std::vector<uint8_t>& frame,
                        const std::vector<uint8_t>& payload,
                        const std::string &hex);

  // Firmware-version announce: FD 05 AA 0B <major> <minor> <patch> <chk>
  void on_fd_version_frame(uint8_t major, uint8_t minor, uint8_t patch);

 private:
  static inline uint16_t read_u16_be(const std::vector<uint8_t> &b, int i) { return uint16_t((b[i]<<8)|b[i+1]); }
  static inline uint32_t read_u32_be(const std::vector<uint8_t> &b, int i) { return (uint32_t(b[i])<<24)|(uint32_t(b[i+1])<<16)|(uint32_t(b[i+2])<<8)|uint32_t(b[i+3]); }
  static inline uint8_t  bcd2dec(uint8_t v) { return uint8_t(((v>>4)*10) + (v & 0x0F)); }
  static const char* status_text(uint8_t raw);

  // Display-Dedupe/Throttle
  std::string last_display_;
  uint32_t    last_display_ms_{0};
  static constexpr uint32_t DISPLAY_MIN_INTERVAL_MS = 5 * 60 * 1000;

  // Fühler-Fehlerwerte (nur bei VERBOSE gewarnt, Wert wird trotzdem publiziert)
  static constexpr uint16_t SENSOR_FAULT_RAW_MIN = 0xFE00;  // unsigned: >= 6502.4 °C
  static constexpr int16_t SENSOR_FAULT_S16_MIN = -300;     // signed: <= -30.0 °C (raw FED4)
  static constexpr int16_t SENSOR_FAULT_S16_MAX = 0x7F00;   // signed: >= 3251.2 °C

  SystaReader &r_;
};

} // namespace systa_reader
} // namespace esphome

#pragma once
#include "systa_reader.h"
#include <cstdint>
#include <string>
#include <vector>

namespace esphome {
namespace systa_reader {

class Expresso2Decoder {
 public:
  explicit Expresso2Decoder(SystaReader &owner) : r_(owner) {}

  void on_fc_frame(const std::vector<uint8_t> &frame,
                   const std::vector<uint8_t> &payload,
                   const std::string &hex);

  // Firmware-version announce: FD 05 AA 14 <major> <minor> <patch> <chk>
  void on_fd_version_frame(uint8_t major, uint8_t minor, uint8_t patch);

 private:
  // Big-Endian helpers (MSB,LSB) — wie bei Espresso/Comfort
  static inline uint8_t read_u8(const std::vector<uint8_t> &b, int i) {
    return (i >= 0 && (size_t) i < b.size()) ? b[i] : 0;
  }
  static inline uint16_t read_u16_be(const std::vector<uint8_t> &b, int i) {
    if (i < 0 || i + 1 >= (int) b.size())
      return 0;
    return uint16_t((b[i] << 8) | b[i + 1]);
  }
  static inline int16_t read_s16_be(const std::vector<uint8_t> &b, int i) {
    return (int16_t) read_u16_be(b, i);
  }

  // "UHR"-Zeit -> "TT.MM HH:MM"
  static void format_timestamp_(uint16_t mins, uint16_t days, char *out,
                                size_t n);

  SystaReader &r_;
};

}  // namespace systa_reader
}  // namespace esphome

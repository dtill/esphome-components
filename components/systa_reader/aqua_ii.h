#pragma once
#include "systa_reader.h"
#include <vector>
#include <string>
#include <cstdint>

namespace esphome {
namespace systa_reader {

class Aqua2Decoder {
 public:
  explicit Aqua2Decoder(SystaReader &owner) : r_(owner) {}

  void on_fc_frame(const std::vector<uint8_t> &frame,
                   const std::vector<uint8_t> &payload,
                   const std::string &hex);

 private:
  // ---- Helpers (zentral, nur hier definiert) ----
  static inline uint8_t bcd2dec(uint8_t v) {
    return uint8_t(((v >> 4) * 10) + (v & 0x0F));
  }
  static inline uint16_t read_u16_le(const std::vector<uint8_t> &b, int i) {
    if (i + 1 >= (int)b.size()) return 0;
    return uint16_t(b[i]) | (uint16_t(b[i + 1]) << 8);
  }
  static inline int16_t read_i16_le(const std::vector<uint8_t> &b, int i) {
    uint16_t v = read_u16_le(b, i);
    return (v >= 0x8000) ? int16_t(v - 0x10000) : int16_t(v);
  }
  static inline uint32_t read_u32_le(const std::vector<uint8_t> &b, int i) {
    if (i + 3 >= (int)b.size()) return 0;
    return (uint32_t)b[i]
         | ((uint32_t)b[i + 1] << 8)
         | ((uint32_t)b[i + 2] << 16)
         | ((uint32_t)b[i + 3] << 24);
  }

  static const char *status_text(uint8_t raw);

  SystaReader &r_;
};

}  // namespace systa_reader
}  // namespace esphome

#pragma once
#include "systa_reader.h"
#include <vector>
#include <string>

namespace esphome {
namespace systa_reader {

class EspressoDecoder {
 public:
  explicit EspressoDecoder(SystaReader &owner) : r_(owner) {}

  void on_fc_frame(const std::vector<uint8_t>& frame,
                   const std::vector<uint8_t>& payload,
                   const std::string &hex);

 private:
  static inline uint16_t read_u16_be(const std::vector<uint8_t> &b, int i) {return static_cast<uint16_t>((b[i] << 8) | b[i+1]);}
  static inline uint8_t bcd2dec(uint8_t v) { return uint8_t(((v>>4)*10) + (v & 0x0F)); }

  SystaReader &r_;
};

}  // namespace systa_reader
}  // namespace esphome

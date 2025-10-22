#pragma once
#include "../systa_reader.h"

namespace esphome {
namespace systa_reader {

class AquaDevice : public DeviceBase {
 public:
  using DeviceBase::DeviceBase;
  void on_fc_frame(const std::vector<uint8_t>& frame,
                   const std::vector<uint8_t>& payload,
                   const std::string &hex) override;
};

}  // namespace systa_reader
}  // namespace esphome

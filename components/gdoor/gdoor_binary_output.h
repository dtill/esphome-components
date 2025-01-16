#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_output/binary_output.h"

namespace esphome {
namespace gdoor_esphome {

class GDoorBinaryOutput : public binary_output::BinaryOutput, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
};

}  // namespace gdoor_esphome
}  // namespace esphome
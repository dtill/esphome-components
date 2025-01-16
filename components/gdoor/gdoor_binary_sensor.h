#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace gdoor_esphome {

class GDoorBinarySensor : public binary_sensor::BinarySensor, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
};

}  // namespace gdoor_esphome
}  // namespace esphome
#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace gdoor_esphome {

class GDoorBusMessage : public text_sensor::TextSensor, public Component {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
};

}  // namespace gdoor_esphome
}  // namespace esphome
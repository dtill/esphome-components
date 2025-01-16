#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace gdoor_esphome {

class GDoorTextSensor : public text_sensor::TextSensor, public Component {
 public:
  void set_parent(GdoorComponent *parent) { this->parent_ = parent; }
  void setup() override;
  void loop() override;
  void dump_config() override;
 protected:
  GdoorComponent *parent_{nullptr};
};

}  // namespace gdoor_esphome
}  // namespace esphome
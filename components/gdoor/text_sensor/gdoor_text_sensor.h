#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "../gdoor_component.h"

namespace esphome {
namespace gdoor_esphome {

class GDoorBusMessage : public text_sensor::TextSensor, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  void set_parent(GdoorComponent *parent) { this->parent_ = parent; }

 protected:
  GdoorComponent *parent_{nullptr};
  bool new_data_available_ = false; // Flag to indicate new data is available
};

}  // namespace gdoor_esphome
}  // namespace esphome
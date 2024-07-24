#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace mcp45hvx1 {

class Mcp45hvx1Output : public output::FloatOutput, public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void write_state(float state) override;
  void set_initial_value(float value) { this->initial_value_ = value; }
  void set_shdn_pin(GPIOPin *pin) { shdn_pin_ = pin; }
  void set_wlat_pin(GPIOPin *pin) { wlat_pin_ = pin; }

  float Mcp45hvx1Output::read_wiper();

 protected:
  float initial_value_;
  GPIOPin *shdn_pin_{nullptr};
  GPIOPin *wlat_pin_{nullptr};
};

}  // namespace mcp45hvx1
}  // namespace esphome

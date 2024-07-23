#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace mcp45hvx1 {

class Mcp45hvx1Output : public output::FloatOutput, public Component, public i2c::I2CDevice {
 public:
  void set_shdn_pin(InternalGPIOPin *pin) { shdn_pin_ = pin; }
  void set_wlat_pin(InternalGPIOPin *pin) { wlat_pin_ = pin; }
  void set_initial_value(float initial_value) { initial_value_ = initial_value; }

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::IO; }
  void update();

  void test_wlat(int change_amount);

 protected:
  void write_state(float state) override;
  InternalGPIOPin *shdn_pin_;
  InternalGPIOPin *wlat_pin_;
  float initial_value_;
  float pot_value_;
};

}  // namespace mcp45hvx1
}  // namespace esphome
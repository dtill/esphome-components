#include "mcp45hvx1.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mcp45hvx1 {

static const char *const TAG = "mcp45hvx1.output";

void Mcp45hvx1Output::test_wlat(int change_amount) {
  if (change_amount == 0) {
    return;
  }
  if (change_amount > 0) {
    this->wlat_pin_->digital_write(true);
  } else {
    this->wlat_pin_->digital_write(false);
  }
}

void Mcp45hvx1Output::setup() {
  ESP_LOGCONFIG(TAG, "Setting up MCP45HVX1 Potentiometer with initial value of %f", this->initial_value_);
}

void X9cOutput::write_state(float state) {
  return;
}

void Mcp45hvx1Output::dump_config() {
  ESP_LOGCONFIG(TAG, "MCP45HVX1 Potentiometer Output:");
  LOG_PIN("  SHDN Pin: ", this->shdn_pin_);
  LOG_PIN("  WLAT Pin: ", this->wlat_pin_);
  ESP_LOGCONFIG(TAG, "  Initial Value: %f", this->initial_value_);
  LOG_FLOAT_OUTPUT(this);
}

}  // namespace mcp45hvx1
}  // namespace esphome
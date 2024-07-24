#include "mcp45hvx1.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mcp45hvx1 {

static const char *TAG = "mcp45hvx1";

void Mcp45hvx1Output::setup() {
  ESP_LOGCONFIG(TAG, "Setting up MCP45HVX1...");
  // Initialization code
}

void Mcp45hvx1Output::write_state(float state) {
  uint8_t int_state = static_cast<int>(state * 255);  // Convert state to 0-255 range
  ESP_LOGD(TAG, "Setting MCP45HVX1 to %d", int_state);

  if (this->is_failed()) {
    ESP_LOGE(TAG, "I2C device not initialized");
    return;
  }

  uint8_t data[2];
  data[0] = 0x00;  // MEM_WIPER | COM_WRITE (fixed value)
  data[1] = int_state;

  i2c::ErrorCode error = this->write(data, 2);
  if (error != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to write to MCP45HVX1: %d", error);
    this->mark_failed();
  }
}


}  // namespace mcp45hvx1
}  // namespace esphome

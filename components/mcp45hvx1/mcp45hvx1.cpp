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
  uint8_t int_state = static_cast<uint8_t>(state * 255);  // Convert state to 0-255 range
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
  } else {
    float read_value = this->read_wiper();
    uint8_t read_int_value = static_cast<uint8_t>(read_value * 255);
    if (read_int_value != int_state) {
      ESP_LOGE(TAG, "Mismatch: Written value %d does not match read value %d", int_state, read_int_value);
    }
  }
}


float Mcp45hvx1Output::read_wiper() {
  if (this->is_failed()) {
    ESP_LOGE(TAG, "I2C device not initialized");
    return -1.0f;  // Return an invalid value
  }

  uint8_t command = 0x0C;  // MEM_WIPER | COM_READ
  i2c::ErrorCode error = this->write(&command, 1);
  if (error != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to send read command to MCP45HVX1: %d", error);
    this->mark_failed();
    return -1.0f;  // Return an invalid value
  }

  uint8_t data[2] = {0};
  error = this->read(data, 2);
  if (error != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to read from MCP45HVX1: %d", error);
    this->mark_failed();
    return -1.0f;  // Return an invalid value
  }

  float state = static_cast<float>(data[1]) / 255.0;  // Convert to float between 0 and 1
  ESP_LOGI(TAG, "Read MCP45HVX1 wiper value: %d", static_cast<int>(read_value * 255));
  return state;
}


}  // namespace mcp45hvx1
}  // namespace esphome

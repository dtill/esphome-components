#include "mcp41hvx1.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mcp41hvx1 {

static const char *TAG = "mcp41hvx1";

void Mcp41hvx1Output::setup() {
  ESP_LOGCONFIG(TAG, "Setting up MCP41HVX1...");
  this->spi_setup();
}

void Mcp41hvx1Output::write_state(float state) {
  uint8_t int_state = static_cast<uint8_t>(state * 255);  // Convert state to 0-255 range
  ESP_LOGD(TAG, "Setting MCP41HVX1 to %d", int_state);

  if (this->is_failed()) {
    ESP_LOGE(TAG, "SPI device MCP41HVX1 not initialized");
    return;
  }

  this->wiper_set_position(int_state);
}

void Mcp41hvx1Output::dump_config() {
  ESP_LOGCONFIG(TAG, "Mcp41hvx1_spi:");
  LOG_PIN("  CS Pin: ", this->cs_pin_);
}

float Mcp41hvx1Output::read_wiper() {
  if (this->is_failed()) {
    ESP_LOGE(TAG, "SPI device not initialized");
    return -1.0f;  // Return an invalid value
  }

  uint8_t position = this->wiper_get_position();

  float state = static_cast<float>(position) / 255.0;  // Convert to float between 0 and 1
  ESP_LOGI(TAG, "Read MCP45HVX1 wiper value: %d", static_cast<int>(state * 255));
  return state;
}

void Mcp41hvx1Output::wiper_set_position(uint8_t position) {
  this->enable();
  this->spi_->transfer(0x00);       // The command for wiper set position
  this->spi_->transfer(position);   // The value for the wiper position
  this->disable();
}

uint8_t Mcp41hvx1Output::wiper_get_position() {
  uint16_t response = 0;
  this->enable();
  response = this->spi_->transfer16(0x0C00);        // requesting the wiper position
  this->disable();
  return static_cast<uint8_t>(response & 0xff);     // The wiper position is in the low order byte
}

}  // namespace mcp45hvx1
}  // namespace esphome

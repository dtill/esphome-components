#include "mcp41hvx1.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mcp41hvx1 {

static const char *TAG = "mcp41hvx1";

// Correctly associating setup function with Mcp41hvx1Output class
void Mcp41hvx1Output::setup() {
  ESP_LOGCONFIG(TAG, "Setting up MCP41HVX1...");
  this->spi_setup();  // Setup the SPI device
}

// Correctly associating write_state function with Mcp41hvx1Output class
void Mcp41hvx1Output::write_state(float state) {
  uint8_t int_state = static_cast<uint8_t>(state * 255);  // Convert state to 0-255 range
  ESP_LOGD(TAG, "Setting MCP41HVX1 to %d", int_state);

  if (this->is_failed()) {
    ESP_LOGE(TAG, "SPI device MCP41HVX1 not initialized");
    return;
  }

  this->wiper_set_position(int_state);
}

// Correctly associating dump_config function with Mcp41hvx1Output class
void Mcp41hvx1Output::dump_config() {
  ESP_LOGCONFIG(TAG, "Mcp41hvx1_spi:");
  LOG_PIN("  CS Pin: ", cs_);
  ESP_LOGCONFIG(TAG, "  Data Rate: %d", this->data_rate_);
  ESP_LOGCONFIG(TAG, "  Mode: %d", this->mode_);
  ESP_LOGCONFIG(TAG, "  Bit Order: %d", this->bit_order_);
}

// Correctly associating wiper_set_position function with Mcp41hvx1Output class
void Mcp41hvx1Output::wiper_set_position(uint8_t position) {
  this->enable();  // Assert CS low to start communication

  this->write_byte(0x00);  // The command for wiper set position
  this->write_byte(position);  // The value for the wiper position

  this->disable();  // Assert CS high to end communication
}

// Correctly associating wiper_get_position function with Mcp41hvx1Output class
uint8_t Mcp41hvx1Output::wiper_get_position() {
  this->enable();  // Assert CS low to start communication

  this->write_byte(0x0C);  // The command for requesting the wiper position
  uint8_t position = this->read_byte();  // Read the wiper position

  this->disable();  // Assert CS high to end communication

  return position;
}

}  // namespace mcp41hvx1
}  // namespace esphome

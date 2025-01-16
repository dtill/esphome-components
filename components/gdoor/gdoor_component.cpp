#include "gdoor_component.h"
#include "esphome/core/log.h"

namespace esphome {
namespace gdoor_esphome {

static const char *TAG = "gdoor_esphome.gdoor_component";

void GdoorComponent::set_tx_pin(GPIOPin *tx_pin) {
  this->tx_pin_ = tx_pin;
}

void GdoorComponent::set_tx_en_pin(GPIOPin *tx_en_pin) {
  this->tx_en_pin_ = tx_en_pin;
}

void GdoorComponent::set_rx_pin(GPIOPin *rx_pin) {
  this->rx_pin_ = rx_pin;
}

void GdoorComponent::set_rx_sens(float rx_sens) {
  this->rx_sens_ = rx_sens;
}

void GdoorComponent::setup() {
  ESP_LOGI(TAG, "Setting up GdoorComponent");
  ESP_LOGI(TAG, "Configuring GDoor bus pins: TX=%d, TX_EN=%d, RX=%d", this->tx_pin_, this->tx_en_pin_, this->rx_pin_);
  GDOOR::setup(PIN_TX, PIN_TX_EN, RX_PIN_22_NUM);

  if (this->rx_pin_ == 22 && this->rx_sens_ != 1.65) {
    ESP_LOGI(TAG, "Setting RX threshold to %f", RX_SENS_MED_NUM);
    GDOOR::setRxThreshold(PIN_RX_THRESH, RX_SENS_MED_NUM);
  }
}

void GdoorComponent::loop() {
  GDOOR::loop();
  GDOOR_DATA* rx_data = GDOOR::read();
  if (rx_data != nullptr) {
    GDOOR_DATA_PROTOCOL busmessage = GDOOR_DATA_PROTOCOL(rx_data);
    std::string action = busmessage.action;
    this->set_last_rx_data(rx_data);
    ESP_LOGD(TAG, "Received data from GDoor bus: %s", "");
  }
}

}  // namespace gdoor_esphome
}  // namespace esphome

#include "esphome/core/log.h"
#include "gdoor_action_sensor.h"

namespace esphome {
namespace gdoor_esphome {

static const char *TAG = "gdoor_esphome.action_sensor";

void GDoorActionSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up GDoorActionSensor...");
  if (this->parent_ != nullptr) {
    this->parent_->setup();
  }
  this->publish_state(false);
}

void GDoorActionSensor::loop() {
  if (this->parent_ != nullptr) {
    uint32_t parent_timestamp = this->parent_->get_last_bus_update();
    if (parent_timestamp != this->last_bus_update_) {
      std::string last_message = this->parent_->get_last_rx_data_str();
      if (!this->busdata_.empty() && last_message.find(this->busdata_) != std::string::npos) {
        ESP_LOGVV(TAG, "Matched busdata: %s", this->busdata_.c_str());
        this->publish_state(true);
        this->publish_state(false);
      } else {
        this->publish_state(false);
      }
      this->last_bus_update_ = parent_timestamp;
    }
  } else {
    ESP_LOGW(TAG, "Parent component not set!");
  }
}

void GDoorActionSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "GDoor Action Sensor binary_sensor");
  ESP_LOGCONFIG(TAG, "Busdata filter: %s", this->busdata_.c_str());
}

}  // namespace gdoor_esphome
}  // namespace esphome

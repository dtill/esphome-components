#include "esphome/core/log.h"
#include "gdoor_text_sensor.h"

namespace esphome {
namespace gdoor_esphome {

static const char *TAG = "gdoor_esphome.text_sensor";

void GdoorTextSensor::setup() {
  ESP_LOGI(TAG, "Setting up GdoorTextSensor");
  if (this->parent_ != nullptr) {
    this->parent_->setup();
  }
  publish_state("BUS_IDLE");
}

void GdoorTextSensor::loop() {
  if (this->parent_ != nullptr) {
    this->parent_->loop();
    std::string last_data = this->parent_->get_last_rx_data();
    if (!last_data.empty()) {
      ESP_LOGI(TAG, "Publishing updated data: %s", last_data.c_str());
      publish_state(last_data);
    }
  }
}

void GdoorTextSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "Gdoor Text Sensor:");
  if (this->parent_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Parent GdoorComponent id: %s", this->parent_->get_id().c_str());
  } else {
    ESP_LOGCONFIG(TAG, "  Parent GdoorComponent not set");
  }
}

}  // namespace gdoor_esphome
}  // namespace esphome

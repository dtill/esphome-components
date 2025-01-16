#include "esphome/core/log.h"
#include "gdoor_text_sensor.h"

namespace esphome {
namespace gdoor_esphome {

static const char *TAG = "gdoor_esphome.text_sensor";

void GDoorBusMessage::setup() {
  ESP_LOGI(TAG, "Setting up GDoorBusMessage text sensor");
  if (this->parent_ != nullptr) {
    this->parent_->setup();
  }
  publish_state("BUS_IDLE");
}

void GDoorBusMessage::loop() {
  if (this->parent_ != nullptr) {
    //this->parent_->loop();
    std::string last_message = this->parent_->get_last_rx_data_str();
    if (!last_message.empty()) {
      ESP_LOGD(TAG, "Publishing updated bus message: %s", last_message.c_str());
      publish_state(last_message.c_str());
    }
  }
}

void GDoorBusMessage::dump_config() {
  ESP_LOGCONFIG(TAG, "GDoor Bus Message text sensor");
}

}  // namespace gdoor_esphome
}  // namespace esphome

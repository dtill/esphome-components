#include "esphome/core/log.h"
#include "gdoor_bus_write.h"

namespace esphome {
namespace gdoor_esphome {

static const char *TAG = "gdoor_esphome.bus_write";

void GDoorBusWrite::write_state(bool state) {
  if (this->parent_ == nullptr) {
    ESP_LOGW(TAG, "Parent component is not set, cannot write to bus");
    return;
  }
  ESP_LOGVV(TAG, "Writing state: %s", state ? "ON" : "OFF");
  ESP_LOGVV(TAG, "  Sending payload: %s", this->payload_.c_str());
  this->parent_->send_bus_message(this->payload_);
}

void GDoorBusWrite::dump_config() {
  ESP_LOGCONFIG(TAG, "GDoor Bus Writer:");
  ESP_LOGCONFIG(TAG, "  Payload: %s", this->payload_.c_str());
  ESP_LOGCONFIG(TAG, "  Require Response: %s", this->require_response_ ? "YES" : "NO");
}

}  // namespace gdoor_esphome
}  // namespace esphome

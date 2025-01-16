#include "esphome/core/log.h"
#include "gdoor_binary_output.h"
#include "gdoor.h"

namespace esphome {
namespace gdoor_esphome {

static const char *TAG = "gdoor_esphome.gdoor";
static const GDOOR_DATA_PROTOCOL gdoor_data_idle(NULL, true);

void GDoorBinaryOutput::setup() {
    ESP_LOGI(TAG, "Setting up GDoorBinaryOutput");
}

void GDoorBinaryOutput::dump_config() {
    ESP_LOGCONFIG(TAG, "GDoor binary output");
}

void GDoorBinaryOutput::loop() {
    GDOOR::loop();

    GDOOR_DATA* rx_data = GDOOR::read();
    if(rx_data != NULL) {
        ESP_LOGI(TAG, "Received data from bus");
        GDOOR_DATA_PROTOCOL busmessage = GDOOR_DATA_PROTOCOL(rx_data);

        ESP_LOGD(TAG, "Data: ", busmessage);

        // Set sensor state.
        publish_state(busmessage.action);

        ESP_LOGD(TAG, "Sensor state set. Back to idle.");

        // TODO: Set sensor state back to idle.
        publish_state(gdoor_data_idle.action);
    }
}

}  // namespace gdoor_esphome
}  // namespace esphome
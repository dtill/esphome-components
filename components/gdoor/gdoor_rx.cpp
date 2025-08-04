#include "defines.h"
#include "gdoor_rx.h"
#include "gdoor_data.h"
#include "esphome/core/log.h"

static const char *TAG = "gdoor_esphome.gdoor_rx";
namespace GDOOR_RX {
    static volatile bool isr_was_called = false;
    uint8_t pin_rx = 0;
    GDOOR_DATA retval; uint16_t rx_state = 0;
    void ARDUINO_ISR_ATTR isr_extint_rx() { isr_was_called = true; }
    void setup(uint8_t rxpin) {
        pin_rx = rxpin;
        pinMode(pin_rx, INPUT_PULLUP);
        ESP_LOGI(TAG, "RX setup on pin %d. Attaching interrupt.", pin_rx);
        attachInterrupt(pin_rx, isr_extint_rx, FALLING);
    }
    void loop() {
        if (isr_was_called) {
            ESP_LOGI(TAG, "SUCCESS: RX Interrupt was triggered!");
            isr_was_called = false;
        }
    }
    GDOOR_DATA* read() { return NULL; }
}
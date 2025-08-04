#include "defines.h"
#include "gdoor_rx.h"
#include "gdoor_data.h"
#include "gdoor_utils.h"
#include "esphome/core/log.h"

static const char *TAG = "gdoor_esphome.gdoor_rx";

namespace GDOOR_RX {

    // --- DEBUGGING FLAG ---
    static volatile bool isr_was_entered_at_all = false;

    static volatile uint32_t timings_buffer[MAX_WORDLEN * 20] = {0};
    static volatile int32_t timings_buffer_pos = -1;
    hw_timer_t *stopwatch = NULL;
    uint8_t pin_rx = 0;
    GDOOR_DATA retval;
    uint16_t rx_state = 0;

    void ARDUINO_ISR_ATTR isr_extint_rx() {
        // Die ALLERERSTE Aktion in der ISR ist, diesen Flag zu setzen.
        // Das ist die sicherste Operation, die es gibt.
        isr_was_entered_at_all = true;

        // Der Rest der Logik, die wahrscheinlich abstürzt:
        if (stopwatch != NULL && timings_buffer_pos < ((MAX_WORDLEN * 20) - 1)) {
            timings_buffer_pos++;
            timings_buffer[timings_buffer_pos] = timerRead(stopwatch);
        }
    }

    void enable() { attachInterrupt(pin_rx, isr_extint_rx, FALLING); }
    void disable() { detachInterrupt(pin_rx); }

    void setup(uint8_t rxpin) {
        // --- DEBUGGING LOG ---
        ESP_LOGI(TAG, "MICROSCOPIC TEST: Complex RX setup is starting...");
        pin_rx = rxpin;
        pinMode(pin_rx, INPUT_PULLUP);

        stopwatch = timerBegin(1000000);
        timerStart(stopwatch);

        timings_buffer_pos = -1;
        ESP_LOGI(TAG, "MICROSCOPIC TEST: Stopwatch started. Attaching interrupt.");
        enable();
    }

    void loop() {
        // --- DEBUGGING CHECK ---
        if (isr_was_entered_at_all) {
            ESP_LOGE(TAG, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
            ESP_LOGE(TAG, "!!! SUCCESS: The ISR was ENTERED! System crashed AFTER this point.");
            ESP_LOGE(TAG, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
            isr_was_entered_at_all = false;
        }

        // Der Rest der normalen Loop-Logik
        if (timings_buffer_pos < 0) return;
        if ((timerRead(stopwatch) - timings_buffer[timings_buffer_pos]) > 5000) {
            // ... (normale Verarbeitungslogik, wird wahrscheinlich nie erreicht)
            ESP_LOGI(TAG, "Message timeout detected. This should not be reached yet.");
            timings_buffer_pos = -1;
        }
    }

    GDOOR_DATA* read() { return NULL; }
}
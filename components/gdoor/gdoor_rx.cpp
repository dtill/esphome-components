#include "defines.h"
#include "gdoor_rx.h"
#include "gdoor_data.h"
#include "gdoor_utils.h"
#include "esphome/core/log.h"

static const char *TAG = "gdoor_esphome.gdoor_rx";

namespace GDOOR_RX {

    // Buffer to store microsecond timestamps of each interrupt
    static volatile uint32_t timings_buffer[MAX_WORDLEN * 20] = {0};
    static volatile int32_t timings_buffer_pos = -1;

    uint8_t pin_rx = 0;
    GDOOR_DATA retval;
    uint16_t rx_state = 0; // Kept for API compatibility

    // ISR is now extremely simple and uses the standard, safe micros() function.
    void ARDUINO_ISR_ATTR isr_extint_rx() {
        if (timings_buffer_pos < ((MAX_WORDLEN * 20) - 1)) {
            timings_buffer_pos++;
            timings_buffer[timings_buffer_pos] = micros();
        }
    }

    // These functions are required by gdoor_tx.cpp
    void enable() {
        attachInterrupt(pin_rx, isr_extint_rx, FALLING);
        ESP_LOGD(TAG, "RX interrupt enabled.");
    }
    void disable() {
        detachInterrupt(pin_rx);
        ESP_LOGD(TAG, "RX interrupt disabled.");
    }

    void setup(uint8_t rxpin) {
        pin_rx = rxpin;
        pinMode(pin_rx, INPUT_PULLUP);
        timings_buffer_pos = -1;
        // No timer setup needed anymore!
        ESP_LOGI(TAG, "FINAL RX: Setup on pin %d using ISR-safe micros() method.", rxpin);
        enable();
    }

    void loop() {
        // 1. Check if any data has been received at all
        if (timings_buffer_pos < 0) {
            return;
        }

        // 2. Check for a message timeout. If more than ~5ms have passed since the last pulse,
        //    we assume the message is complete.
        if ((micros() - timings_buffer[timings_buffer_pos]) > 5000) {

            // --- Safely copy the buffer and reset the ISR for the next message ---
            noInterrupts();
            int32_t local_pos = timings_buffer_pos;
            uint32_t local_timings[MAX_WORDLEN * 20];
            if (local_pos >= 0) {
                memcpy(local_timings, (void *)timings_buffer, (local_pos + 1) * sizeof(uint32_t));
            }
            timings_buffer_pos = -1;
            interrupts();

            if (local_pos < 1) return;

            ESP_LOGD(TAG, "RX message complete with %d timings. Reconstructing counts...", local_pos + 1);

            // --- 3. Reconstruct the original `counts` array from µs timings ---
            uint16_t counts[MAX_WORDLEN * 9] = {0};
            uint8_t bit_idx = 0;
            uint16_t pulse_count = 1;
            const uint32_t PULSE_MIN_DELTA_US = 10;
            const uint32_t PULSE_MAX_DELTA_US = 30;

            for (int i = 0; i < local_pos; i++) {
                uint32_t delta = local_timings[i+1] - local_timings[i];
                if (delta >= PULSE_MIN_DELTA_US && delta <= PULSE_MAX_DELTA_US) {
                    pulse_count++;
                } else {
                    if (bit_idx < (MAX_WORDLEN * 9)) counts[bit_idx++] = pulse_count;
                    pulse_count = 1;
                }
            }
            if (bit_idx < (MAX_WORDLEN * 9)) counts[bit_idx++] = pulse_count;

            // --- 4. DEBUG OUTPUT and PARSING ---
            char buffer[256];
            int offset = 0;
            offset += snprintf(buffer, sizeof(buffer), "Reconstructed Counts: [");
            for(int i=0; i<bit_idx; i++) {
                if(offset < 240) offset += snprintf(buffer+offset, sizeof(buffer)-offset, "%d, ", counts[i]);
            }
            snprintf(buffer+offset, sizeof(buffer)-offset, "]");
            ESP_LOGD(TAG, "%s", buffer);

            if (retval.parse(counts, bit_idx)) {
                ESP_LOGI(TAG, "Parse SUCCESS!");
                rx_state |= FLAG_DATA_READY;
            } else {
                ESP_LOGW(TAG, "Parse FAILED!");
            }
        }
    }

    GDOOR_DATA* read() {
        if (rx_state & FLAG_DATA_READY) {
            rx_state &= (uint16_t)~FLAG_DATA_READY;
            return &retval;
        }
        return NULL;
    }
}
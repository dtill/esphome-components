/*
 * This file is part of the GDoor distribution (https://github.com/gdoor-org).
 * Copyright (c) 2024 GDoor authors.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "defines.h"
#include "gdoor_rx.h"
#include "gdoor_data.h"
#include "esphome/core/log.h"

static const char *TAG = "gdoor_esphome.gdoor_rx";
namespace GDOOR_RX {
    static volatile uint32_t edge_timings[MAX_WORDLEN * 20] = {0};
    static volatile int32_t edge_pos = -1;
    uint8_t pin_rx = 0;
    GDOOR_DATA retval;
    uint16_t rx_state = 0;

    void ARDUINO_ISR_ATTR isr_extint_rx() {
        if (edge_pos < ((MAX_WORDLEN * 20) - 1)) {
            edge_pos++;
            edge_timings[edge_pos] = micros();
        }
    }

    void enable() { attachInterrupt(pin_rx, isr_extint_rx, FALLING); }
    void disable() { detachInterrupt(pin_rx); }
    void reset() { edge_pos = -1; rx_state = 0; }

    void setup(uint8_t rxpin) {
        pin_rx = rxpin;
        pinMode(pin_rx, INPUT_PULLUP);
        reset();
        enable();
    }

    void loop() {
        if (edge_pos < 0) return;
        if ((micros() - edge_timings[edge_pos]) > 5000) {
            noInterrupts();
            int32_t local_pos = edge_pos;
            uint32_t local_timings[MAX_WORDLEN * 20];
            if (local_pos >= 0) memcpy(local_timings, (void *)edge_timings, (local_pos + 1) * sizeof(uint32_t));
            edge_pos = -1;
            interrupts();

            if (local_pos < 1) return;

            uint16_t counts[MAX_WORDLEN * 9] = {0};
            uint8_t bit_idx = 0;
            uint16_t current_pulse_count = 0;
            const uint32_t PAUSE_BETWEEN_BITS_US = 150;

            for (int i = 0; i <= local_pos; i++) {
                current_pulse_count++;
                bool is_last_pulse = (i == local_pos);
                if (!is_last_pulse) {
                    uint32_t delta_to_next = local_timings[i+1] - local_timings[i];
                    if (delta_to_next > PAUSE_BETWEEN_BITS_US) {
                        if (bit_idx < (MAX_WORDLEN * 9)) counts[bit_idx++] = current_pulse_count;
                        current_pulse_count = 0;
                    }
                } else {
                    if (bit_idx < (MAX_WORDLEN * 9)) counts[bit_idx++] = current_pulse_count;
                }
            }

            // Temporäres Debugging, um das rekonstruierte Array zu sehen
            char debug_buffer[256];
            int offset = 0;
            offset += snprintf(debug_buffer, sizeof(debug_buffer), "Reconstructed Counts: [");
            for(int i=0; i<bit_idx; i++) {
                if(offset < 240) offset += snprintf(debug_buffer+offset, sizeof(debug_buffer)-offset, "%d, ", counts[i]);
            }
            snprintf(debug_buffer+offset, sizeof(debug_buffer)-offset, "]");
            ESP_LOGD(TAG, "%s", debug_buffer);
            // Ende Debugging

            if (retval.parse(counts, bit_idx)) {
                rx_state |= FLAG_DATA_READY;
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
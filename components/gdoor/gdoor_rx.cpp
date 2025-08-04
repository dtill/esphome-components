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
#include "esphome/core/log.h" // Wird nur für TAG benötigt, kann aber bleiben

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

    void enable() { attachInterrupt(pin_rx, isr_extint_rx, CHANGE); }
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

            if (retval.parse_from_timings(local_timings, local_pos + 1)) {
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
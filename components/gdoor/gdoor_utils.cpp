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
#include "gdoor_utils.h"

#ifndef APB_CLK_FREQ
// Fallback for ESP32 APB clock frequency (80 MHz) if not defined by the SDK.
#define APB_CLK_FREQ 80000000U
#endif

namespace GDOOR_UTILS {

    uint8_t crc(uint8_t *words, uint16_t len) {
        uint8_t crc = 0;
        // Simple checksum: sum of all bytes
        for (uint16_t i = 0; i < len; i++) {
            crc = static_cast<uint8_t>(crc + words[i]);
        }
        return crc;
    }

    uint8_t parity_odd(uint8_t word) {
        uint8_t ones = 0;
        // Count the number of set bits (Hamming weight)
        while (word != 0) {
            ones++;
            word = static_cast<uint8_t>(word & (word - 1));
        }
        // If the number of set bits is odd, least significant bit is 1
        return static_cast<uint8_t>(ones & 0x01);
    }

    uint16_t divider(uint32_t frequency) {
        uint16_t divider = 0;
        if (frequency > 0) {
            // Calculate divider for the given frequency based on APB clock
            divider = static_cast<uint16_t>(APB_CLK_FREQ / frequency);
        }
        // Limit divider to the valid range for 16-bit hardware
        if (divider < 2 || divider > 65535) {
            divider = 0;
        }
        return divider;
    }

}  // namespace GDOOR_UTILS
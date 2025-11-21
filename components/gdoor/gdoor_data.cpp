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
#include <map>
#include "defines.h"
#include "gdoor_data.h"
#include "gdoor_utils.h"

// Map the HW Type field between bus value and human readable string
static const std::map<int, const char*> GDOOR_DATA_HWTYPE = {
    { 0xA0, "OUTDOOR" },
    { 0xA1, "INDOOR" },
    { 0xA2, "INDOOR_RECEIVER" },
    { 0xA3, "CONTROLLER" },
    { 0xA4, "ACTUATOR" },
    { 0xA5, "GATEWAY_TK" },
    { 0xA6, "CHIME" },
    { 0xA7, "BUTTON_IF" },
    { 0xA8, "GATEWAY_IP" },
};

// Map the Action field between bus value and human readable string
static const std::map<int, const char*> GDOOR_DATA_ACTION = {
    { 0x42, "BUTTON" },
    { 0x41, "BUTTON_LIGHT" },
    { 0x31, "DOOR_OPEN" },
    { 0x28, "VIDEO_REQUEST" },
    { 0x21, "AUDIO_REQUEST" },
    { 0x20, "AUDIO_VIDEO_END" },
    { 0x13, "BUTTON_FLOOR" },
    { 0x12, "CALL_INTERNAL" },
    { 0x11, "BUTTON_RING" },
    { 0x0F, "CTRL_DOOROPENER_ACK" },
    { 0x08, "CTRL_RESET" },
    { 0x05, "CTRL_DOORSTATION_ACK" },
    { 0x04, "CTRL_BUTTONS_TRAINING_START" },
    { 0x03, "CTRL_DOOROPENER_TRAINING_START" },
    { 0x02, "CTRL_DOOROPENER_TRAINING_STOP" },
    { 0x01, "CTRL_PROGRAMMING_START" },
    { 0x00, "CTRL_PROGRAMMING_STOP" }
};

/**
 * Parse function, reading the raw timer count values
 * and populating the GDOOR_DATA fields.
 *
 * @param counts Array with pulse counts of bits.
 * @param len Number of elements in the array.
 * @return true if parsing was successful.
 */
bool GDOOR_DATA::parse(uint16_t *counts, uint16_t len) {
    uint8_t  wordcounter             = 0;  // Current word index
    uint8_t  current_pulsetrain_valid = 1; // Set to 0 if parity or CRC fails
    uint16_t bit_one_thres           = 0;  // Dynamic threshold between bit 0/1 based on start pulse length

    uint8_t is_startbit = 1;      // Flag: current bit is start bit (used to determine thresholds)
    uint8_t bitindex    = 0;      // Current bit index within current word (0..8, 9 bits per word)

    bool success = false;

    for (uint8_t i = 0; i < len; i++) {
        uint16_t cnt = counts[i];
        uint8_t  bit = 0;
        this->raw[i] = cnt;

        // Filter out very small pulses, just ignore them
        if (cnt < BIT_MIN_LEN) {
            continue;
        }

        // Verify that the first start bit is roughly in the expected range
        if (is_startbit && cnt < STARTBIT_MIN_LEN) {
            continue;
        }

        // First bit of a word is a start bit; we use it to determine
        // the length threshold for zero/one bits.
        if (is_startbit) {
            bit_one_thres = static_cast<uint16_t>(cnt / BIT_ONE_DIV);
            is_startbit   = 0;
        } else {
            // Normal bit (not start bit)

            // If we start a new word, clear its content
            if (bitindex == 0) {
                this->data[wordcounter] = 0;
            }

            // Detect zero or one bit value
            if (cnt < bit_one_thres) {
                bit = 1;
            }

            // Parity bit at index 8
            if (bitindex == 8) {
                // Check if parity bit matches expected odd parity
                if (GDOOR_UTILS::parity_odd(this->data[wordcounter]) != bit) {
                    current_pulsetrain_valid = 0;
                }
                bitindex   = 0;
                wordcounter = static_cast<uint8_t>(wordcounter + 1);
            } else {
                // Normal data bits 0..7
                this->data[wordcounter] |= static_cast<uint8_t>(bit << bitindex);
                bitindex = static_cast<uint8_t>(bitindex + 1);
            }

        } // End normal bit
    } // End for

    if (wordcounter != 0) {
        // Check last word for CRC value
        if (GDOOR_UTILS::crc(this->data, static_cast<uint16_t>(wordcounter - 1)) !=
            this->data[wordcounter - 1]) {
            current_pulsetrain_valid = 0;
        }

        this->len   = wordcounter;
        this->valid = current_pulsetrain_valid;
        success     = true;
    }

    return success;
}

/*
 * Constructor for GDOOR_DATA_PROTOCOL:
 * parses the bus data from GDOOR_DATA and stores a
 * human-readable representation in its fields.
 *
 * @param data GDOOR_DATA instance with parsed bus data.
 * @param idle If true, generate an idle message.
 */
GDOOR_DATA_PROTOCOL::GDOOR_DATA_PROTOCOL(GDOOR_DATA *data, bool idle) {
    if (idle) {
        this->type   = "TYPE_GDOOR";
        this->action = "BUS_IDLE";
    } else {
        this->type   = "TYPE_UNKNOWN";
        this->action = "ACTION_UNKNOWN";
    }
    this->raw = data;

    this->source[0] = 0x00;
    this->source[1] = 0x00;
    this->source[2] = 0x00;

    this->destination[0] = 0x00;
    this->destination[1] = 0x00;
    this->destination[2] = 0x00;

    this->parameters[0] = 0x00;
    this->parameters[1] = 0x00;

    if (data != nullptr && data->valid && data->len >= 9) {
        // Hardware type
        auto hw_it = GDOOR_DATA_HWTYPE.find(data->data[8]);
        if (hw_it != GDOOR_DATA_HWTYPE.end()) {
            this->type = hw_it->second;
        }

        // Action type
        auto act_it = GDOOR_DATA_ACTION.find(data->data[2]);
        if (act_it != GDOOR_DATA_ACTION.end()) {
            this->action = act_it->second;
        }

        // Parameters
        this->parameters[0] = data->data[6];
        this->parameters[1] = data->data[7];

        // Source address
        this->source[0] = data->data[3];
        this->source[1] = data->data[4];
        this->source[2] = data->data[5];

        // Destination address (only if present)
        if (data->len >= 12) {
            this->destination[0] = data->data[9];
            this->destination[1] = data->data[10];
            this->destination[2] = data->data[11];
        }
    }
}
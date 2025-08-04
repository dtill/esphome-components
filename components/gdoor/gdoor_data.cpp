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
#include "esphome/core/log.h"   // for logging

// Map the HW Type field between bus value and human readable string
std::map<int, const char*>GDOOR_DATA_HWTYPE = {
    { 0xA0, "OUTDOOR"},
    { 0xA1, "INDOOR"},
    { 0xA2, "INDOOR_RECEIVER"},
    { 0xA3, "CONTROLLER"},
    { 0xA4, "ACTUATOR"},
    { 0xA5, "GATEWAY_TK"},
    { 0xA6, "CHIME"},
    { 0xA7, "BUTTON_IF"},
    { 0xA8, "GATEWAY_IP"},
};

// Map the Action field between bus value and human readable string
std::map<int, const char*>GDOOR_DATA_ACTION = {
    { 0x42, "BUTTON"},
    { 0x41, "BUTTON_LIGHT"},
    { 0x31, "DOOR_OPEN"},
    { 0x28, "VIDEO_REQUEST"},
    { 0x21, "AUDIO_REQUEST"},
    { 0x20, "AUDIO_VIDEO_END"},
    { 0x13, "BUTTON_FLOOR"},
    { 0x12, "CALL_INTERNAL"},
    { 0x11, "BUTTON_RING"},
    { 0x0F, "CTRL_DOOROPENER_ACK"},
    { 0x08, "CTRL_RESET"},
    { 0x05, "CTRL_DOORSTATION_ACK"},
    { 0x04, "CTRL_BUTTONS_TRAINING_START"},
    { 0x03, "CTRL_DOOROPENER_TRAINING_START"},
    { 0x02, "CTRL_DOOROPENER_TRAINING_STOP"},
    { 0x01, "CTRL_PROGRAMMING_START"},
    { 0x00, "CTRL_PROGRAMMING_STOP"}
};

bool GDOOR_DATA::parse_from_timings(uint32_t *timings, uint16_t len) {
    // Pulse duration definitions from protocol, with generous tolerances
    const uint32_t START_BIT_DUR = 1000, BIT_0_DUR = 500, BIT_1_DUR = 250;
    const uint32_t TOLERANCE = 150; // µs

    uint8_t wordcounter = 0;
    uint8_t bitindex = 0;

    // We need at least 2 edges for one pulse
    if (len < 2) return false;

    // Check if the first pulse is a valid Start-Bit
    uint32_t first_pulse_duration = timings[1] - timings[0];
    if (abs(first_pulse_duration - START_BIT_DUR) > TOLERANCE) {
        ESP_LOGW(TAG, "Parse failed: First pulse is not a valid Start-Bit (duration: %d µs)", first_pulse_duration);
        return false;
    }

    // Iterate through the remaining pulses (2 edges at a time)
    for (int i = 2; i < len; i += 2) {
        if (wordcounter >= MAX_WORDLEN) break;

        if (bitindex == 0) {
            this->data[wordcounter] = 0; // Clear byte for new data
        }

        uint32_t pulse_duration = timings[i] - timings[i-1];
        uint8_t bit = 0;

        if (abs(pulse_duration - BIT_0_DUR) < TOLERANCE) {
            bit = 0;
        } else if (abs(pulse_duration - BIT_1_DUR) < TOLERANCE) {
            bit = 1;
        } else {
            ESP_LOGW(TAG, "Parse failed: Unknown bit duration %d µs at bit %d of word %d", pulse_duration, bitindex, wordcounter);
            return false; // Invalid bit, abort parsing
        }

        // Parity Bit
        if (bitindex == 8) {
            if (GDOOR_UTILS::parity_odd(this->data[wordcounter]) != bit) {
                ESP_LOGW(TAG, "Parse failed: Parity check failed for word %d", wordcounter);
                this->valid = 0;
                return false;
            }
            bitindex = 0;
            wordcounter++;
        } else { // Normal Data Bits
            this->data[wordcounter] |= (uint8_t)(bit << bitindex);
            bitindex++;
        }
    }

    if (wordcounter == 0) return false;

    // Final CRC check
    if (GDOOR_UTILS::crc(this->data, wordcounter - 1) != this->data[wordcounter - 1]) {
        ESP_LOGW(TAG, "Parse failed: CRC check failed.");
        this->valid = 0;
        return false;
    }

    this->len = wordcounter;
    this->valid = 1;
    ESP_LOGI(TAG, "Parse SUCCESS! Length: %d bytes.", this->len);
    return true;
}

/*
* Constructor for GDOOR_DATA_PROTOCOL,
* parses the bus data based on GDOOR_DATA,
* and stores a human readable form in its class elements.
*
* @param data GDOOR_DATA element, with parsed bus data.
* @param idle true: generate idle message
*/
GDOOR_DATA_PROTOCOL::GDOOR_DATA_PROTOCOL(GDOOR_DATA* data, bool idle) {
    if(idle) {
        this->type = "TYPE_GDOOR";
        this->action = "BUS_IDLE";
    } else {
        this->type = "TYPE_UNKOWN";
        this->action = "ACTION_UNKOWN";
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

    if(data != NULL && data->valid && data->len >= 9) {
        if(GDOOR_DATA_HWTYPE.find(data->data[8]) != GDOOR_DATA_HWTYPE.end()){
            this->type = GDOOR_DATA_HWTYPE.at(data->data[8]);
        }
        if(GDOOR_DATA_ACTION.find(data->data[2]) != GDOOR_DATA_ACTION.end()){
            this->action = GDOOR_DATA_ACTION.at(data->data[2]);
        }

        this->parameters[0] = data->data[6];
        this->parameters[1] = data->data[7];

        this->source[0] = data->data[3];
        this->source[1] = data->data[4];
        this->source[2] = data->data[5];

        if(data->len >= 12) {
            this->destination[0] = data->data[9];
            this->destination[1] = data->data[10];
            this->destination[2] = data->data[11];
        }
    }
}
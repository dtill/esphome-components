
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
#ifndef GDOOR_DATA_H
#define GDOOR_DATA_H

#include <map>
#include "defines.h"
#include "gdoor_utils.h"

/**
 * @brief Container for low-level bus data.
 */
class GDOOR_DATA {
  public:
    uint16_t len = 0;
    uint8_t  data[MAX_WORDLEN];
    uint16_t raw[MAX_WORDLEN * 9];
    uint8_t  valid = 0;

    /**
     * @brief Parse incoming pulse counts into bus data.
     * @return true if parsing was successful, false otherwise.
     */
    bool parse(uint16_t *counts, uint16_t len);

    /**
     * @brief Append JSON representation to the given string.
     *
     * Example output fragment:
     *   "busdata": "A1B2...", "raw": ["0x...", ...], "valid": true
     *
     * @param out String to append JSON to.
     * @return Number of characters appended.
     */
    std::size_t to_json(std::string &out) const {
        std::size_t r = 0;

        // "busdata": "..."
        r += GDOOR_UTILS::print_json_hexstring<uint8_t>(out, "busdata", data, len);
        out += ", ";
        r += 2;

        // "raw": ["0x...", ...]
        r += GDOOR_UTILS::print_json_hexarray<uint16_t>(out, "raw", raw, len * 9);
        out += ", ";
        r += 2;

        // "valid": true / false
        r += GDOOR_UTILS::print_json_bool<uint8_t>(out, "valid", valid);

        return r;
    }
};

/**
 * @brief Container for high-level decoded protocol information.
 */
class GDOOR_DATA_PROTOCOL {
  public:
    GDOOR_DATA *raw = nullptr;
    const char *type = nullptr;
    const char *action = nullptr;
    uint8_t parameters[2]   = {0, 0};
    uint8_t source[3]       = {0, 0, 0};
    uint8_t destination[3]  = {0, 0, 0};

    GDOOR_DATA_PROTOCOL(GDOOR_DATA *data, bool idle = false);

    /**
     * @brief Append JSON representation to the given string.
     *
     * Example fields:
     *   "action", "parameters", "source", "destination", "type", "busdata", "event_id"
     *
     * @param out String to append JSON to.
     * @return Number of characters appended.
     */
    std::size_t to_json(std::string &out) const {
        std::size_t r = 0;
        static uint32_t cnt = 0;

        // "action": "..."
        r += GDOOR_UTILS::print_json_string(out, "action", action);
        out += ", ";
        r += 2;

        // "parameters": "A1B2"
        r += GDOOR_UTILS::print_json_hexstring<uint8_t>(out, "parameters", parameters, 2);
        out += ", ";
        r += 2;

        // "source": "..."
        r += GDOOR_UTILS::print_json_hexstring<uint8_t>(out, "source", source, 3);
        out += ", ";
        r += 2;

        // "destination": "..."
        r += GDOOR_UTILS::print_json_hexstring<uint8_t>(out, "destination", destination, 3);
        out += ", ";
        r += 2;

        // "type": "..."
        r += GDOOR_UTILS::print_json_string(out, "type", type);

        // Optional raw busdata passthrough
        if (this->raw != nullptr) {
            out += ", ";
            r += 2;
            r += GDOOR_UTILS::print_json_hexstring<uint8_t>(
                out, "busdata", this->raw->data, this->raw->len
            );
        }

        // "event_id": <counter>
        out += ", ";
        r += 2;
        r += GDOOR_UTILS::print_json_value<uint32_t>(out, "event_id", cnt++);

        return r;
    }
};

#endif  // GDOOR_DATA_H
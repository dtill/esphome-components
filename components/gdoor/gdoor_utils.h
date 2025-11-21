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
#ifndef GDOOR_UTILS_H
#define GDOOR_UTILS_H

#include <cstdint>  // uint8_t, uint16_t, uint32_t
#include <cstddef>  // size_t
#include <string>

namespace GDOOR_UTILS {
    uint8_t crc(uint8_t *words, uint16_t len);
    uint8_t parity_odd(uint8_t word);

    uint16_t divider(uint32_t frequency);


    namespace detail {
        inline char hex_digit(uint8_t v) { static const char *digits = "0123456789ABCDEF";return digits[v & 0x0F];}
        inline void append_hex_no_padding(std::string &out, uint8_t v) {
            uint8_t hi = (v >> 4) & 0x0F;
            uint8_t lo = v & 0x0F;
            if (hi != 0) {
                out.push_back(hex_digit(hi));
            }
            out.push_back(hex_digit(lo));
        }
        inline void append_hex_padded(std::string &out, uint8_t v) {
            uint8_t hi = (v >> 4) & 0x0F;
            uint8_t lo = v & 0x0F;
            out.push_back(hex_digit(hi));
            out.push_back(hex_digit(lo));
        }
    } // namespace detail

    /*
    * Template Function (needs to live in header file),
    * used to print out json hex array.
    * 
    * "keyname": {"0xdata[0]", ..., "0xdata[len-1]"}
    */
   template<typename T> size_t print_json_hexarray(std::string &out, const char *keyname, const T *data, const uint16_t len) {
        const size_t start = out.size();
        out.push_back('"');
        out += keyname;
        out += "\": [";
        for (uint16_t i = 0; i < len; i++) {
            out += "\"0x";
            detail::append_hex_no_padding(out, static_cast<uint8_t>(data[i]));
            out.push_back('"');
            if (i != len - 1) {
                out += ", ";
            }
        }
        out.push_back(']');
        return out.size() - start;
   }

   template<typename T> size_t print_json_value(std::string &out, const char *keyname, const T &value) {
        const size_t start = out.size();
        out.push_back('"');
        out += keyname;
        out += "\": \"";
        out += std::to_string(value);
        out.push_back('"');
        return out.size() - start;
   }

   inline size_t print_json_value(std::string &out, const char *keyname, const char *value) {
        const size_t start = out.size();
        out.push_back('"');
        out += keyname;
        out += "\": \"";
        out += value;
        out.push_back('"');
        return out.size() - start;
   }
   inline size_t print_json_value(std::string &out, const char *keyname, const std::string &value) {
        const size_t start = out.size();
        out.push_back('"');
        out += keyname;
        out += "\": \"";
        out += value;
        out.push_back('"');
        return out.size() - start;
   }

   template<typename T> size_t print_json_hexstring(std::string &out, const char *keyname, const T *data, const uint16_t len) {
        const size_t start = out.size();
        out.push_back('"');
        out += keyname;
        out += "\": \"";
        for (uint16_t i = 0; i < len; i++) {
            detail::append_hex_padded(out, static_cast<uint8_t>(data[i]));
        }
        out.push_back('"');
        return out.size() - start;
    }

   template<typename T> size_t print_json_bool(std::string &out, const char *keyname, const T value) {
        const size_t start = out.size();
        out.push_back('"');
        out += keyname;
        out += "\": ";
        out += value ? "true" : "false";
        return out.size() - start;
    }

    inline size_t print_json_string(std::string &out, const char *keyname, const char *value) {
        return print_json_value(out, keyname, value);
    }
}

#endif
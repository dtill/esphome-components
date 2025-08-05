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
#include "gdoor_tx.h"
#include "gdoor_rx.h"
#include "gdoor_utils.h"
#include "esphome/core/log.h"
#include <esp32-hal-ledc.h>

static const char *TAG = "gdoor_esphome.gdoor_tx";

// ---------------------------------------------------------------------------
// configurable timing constants (58 kHz carrier)
// ---------------------------------------------------------------------------
constexpr uint32_t CARRIER_HZ     = 58000;
constexpr uint32_t HALF_WAVE_US   = 1000000UL / CARRIER_HZ;  // ≈17.24µs
constexpr uint16_t START_PULSES   = 60;                      // ≈1ms
constexpr uint16_t ONE_PULSES     = 12;                      // ≈0.21ms
constexpr uint16_t ZERO_PULSES    = 32;                      // ≈0.55ms
constexpr uint32_t PAUSE_US       = 10 * HALF_WAVE_US;       // ≈0.17ms gap
constexpr uint8_t  LEDC_BITS      =  8;                      // 1–14 bits resolution

// ---------------------------------------------------------------------------
// local TX state
// ---------------------------------------------------------------------------
namespace GDOOR_TX {
  enum TX_STATE : uint8_t { IDLE=0, PULSE, GAP };

  struct TxContext {
    uint16_t bit_table[MAX_WORDLEN * 9 + 1];
    uint16_t total_bits = 0;
    uint16_t index      = 0;
    uint32_t deadline_us= 0;
    TX_STATE state      = IDLE;
  } ctx;

  static uint8_t tx_pin_hw = 0;   // GPIO for PWM
  static uint8_t tx_en_hw  = 0;   // GPIO to enable driver

  // helpers
  static inline uint16_t bit2pulses(bool bit) {
    return bit ? ONE_PULSES : ZERO_PULSES;
  }
  static inline uint16_t byte2word(uint8_t byte) {
    uint16_t w = byte;
    if (GDOOR_UTILS::parity_odd(byte)) w |= 0x100;
    return w;
  }

  // must be called in your main loop()
  void loop() {
    if (ctx.state == IDLE) return;
    if ((int32_t)(micros() - ctx.deadline_us) < 0) return;

    if (ctx.state == PULSE) {
      // end of burst → turn off PWM
      ledcWrite(tx_pin_hw, 0);
      ctx.state       = GAP;
      ctx.deadline_us = micros() + PAUSE_US;
      return;
    }
    // GAP state
    if (ctx.index >= ctx.total_bits) {
      // done
      digitalWrite(tx_en_hw, LOW);
      ctx.state = IDLE;
      GDOOR_RX::enable();
      ESP_LOGV(TAG, "TX finished");
      return;
    }
    // next burst
    uint16_t pulses = ctx.bit_table[ctx.index++];
    ledcWrite(tx_pin_hw, 1 << (LEDC_BITS - 1));  // 50% duty
    ctx.deadline_us = micros() + pulses * HALF_WAVE_US;
    ctx.state       = PULSE;
  }

  // public API
  void setup(uint8_t txpin, uint8_t txenpin) {
    tx_pin_hw = txpin;
    tx_en_hw  = txenpin;
    pinMode(tx_en_hw, OUTPUT);
    digitalWrite(tx_en_hw, LOW);

    // single-call LEDC setup: chooses channel automatically
    bool ok = ledcAttach(tx_pin_hw, CARRIER_HZ, LEDC_BITS);
    ESP_LOGCONFIG(TAG, "  LEDC pin       : GPIO %u", tx_pin_hw);
    ESP_LOGCONFIG(TAG, "  Carrier Hz     : %u", CARRIER_HZ);
    ESP_LOGCONFIG(TAG, "  LEDC bits      : %u", LEDC_BITS);
    ESP_LOGCONFIG(TAG, "  LEDC attach OK : %s", ok ? "yes" : "no");

    ctx.state = IDLE;
  }

  void send(uint8_t *data, uint16_t len) {
    if (ctx.state != IDLE || len >= MAX_WORDLEN) return;

    // build the pulse-count table
    ctx.index      = 0;
    ctx.total_bits = 0;
    ctx.bit_table[ctx.total_bits++] = START_PULSES;
    for (uint16_t i = 0; i < len; ++i) {
      uint16_t w = byte2word(data[i]);
      for (uint8_t b = 0; b < 9; ++b)
        ctx.bit_table[ctx.total_bits++] = bit2pulses(w & (1 << b));
    }

    // verbose dump
    #if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_VERBOSE
    {
      char buf[256]; int off = snprintf(buf, sizeof(buf), "Burst table (cnt=%u): [", ctx.total_bits);
      for (uint16_t i = 0; i < ctx.total_bits && off+8 < (int)sizeof(buf); ++i)
        off += snprintf(buf+off, sizeof(buf)-off, "%u,", ctx.bit_table[i]);
      snprintf(buf+off, sizeof(buf)-off, "]");
      ESP_LOGV(TAG, "%s", buf);
    }
    #endif

    // kick off
    GDOOR_RX::disable();
    digitalWrite(tx_en_hw, HIGH);
    ledcWrite(tx_pin_hw, 1 << (LEDC_BITS - 1));
    ctx.deadline_us = micros() + START_PULSES * HALF_WAVE_US;
    ctx.state       = PULSE;
    ESP_LOGV(TAG, "TX started, %u bits", ctx.total_bits - 1);
  }

  void send(String hex) {
    hex.toUpperCase();
    uint8_t buf[MAX_WORDLEN]; uint16_t n=0;
    for (uint16_t i=0; i+1<hex.length() && n<MAX_WORDLEN; i+=2) {
      int hi = strchr("0123456789ABCDEF", hex[i])  - "0123456789ABCDEF";
      int lo = strchr("0123456789ABCDEF", hex[i+1]) - "0123456789ABCDEF";
      if (hi<0||lo<0) break;
      buf[n++] = (hi<<4)|lo;
    }
    if (n) send(buf,n);
  }

  bool busy() { return ctx.state != IDLE; }

}  // namespace GDOOR_TX

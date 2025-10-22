#pragma once
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include <deque>
#include <vector>


namespace esphome {
namespace systa_reader {


class SystaReaderTextSink; // fwd


class SystaReader : public uart::UARTDevice, public Component {
public:
void set_uart(uart::UARTComponent *parent) { this->set_parent(parent); }
void set_log_invalid(bool v) { this->log_invalid_ = v; }


void setup() override {}
void loop() override;
float get_setup_priority() const override { return setup_priority::DATA; }


// Child registration (text sensors subscribe here)
void add_sink(SystaReaderTextSink *sink) { sinks_.push_back(sink); }


protected:
void process_buffer_();
bool try_parse_fc_frame_();
bool try_parse_display_frame_();
static uint8_t checksum_twos_complement_(const std::vector<uint8_t> &data_without_checksum);
static std::string to_hex_(const std::vector<uint8_t> &buf);


std::deque<uint8_t> buf_{};
std::vector<SystaReaderTextSink *> sinks_{};
bool log_invalid_{true};
};


// Minimal sink interface (implemented by text sensor wrapper)
class SystaReaderTextSink {
public:
virtual void publish_frame_hex(const std::string &hex) = 0;
};


} // namespace systa_reader
} // namespace esphome
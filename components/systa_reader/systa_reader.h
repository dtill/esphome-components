#pragma once
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include <deque>
#include <vector>


namespace esphome {
namespace systa_reader {


class SystaReaderTextSink; // forward


class SystaReader : public uart::UARTDevice, public Component {
public:
void set_log_invalid(bool v) { this->log_invalid_ = v; }


void setup() override {}
void loop() override;
float get_setup_priority() const override { return setup_priority::DATA; }


// Textsensor(e) registrieren
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


// Sink-Interface
class SystaReaderTextSink {
public:
virtual void publish_frame_hex(const std::string &hex) = 0;
virtual ~SystaReaderTextSink() = default;
};


// Konkreter Textsensor (bekommt HEX vom Reader)
class SystaReaderTextSensor : public text_sensor::TextSensor, public Component, public SystaReaderTextSink {
public:
void publish_frame_hex(const std::string &hex) override { this->publish_state(hex); }
};


} // namespace systa_reader
} // namespace esphome
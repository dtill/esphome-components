#include "systa_reader.h"
#include "esphome/core/log.h"

namespace esphome {
namespace systa_reader {

static const char *const TAG = "systa_reader";

void SystaReader::loop() {
  uint8_t byte;
  while (this->available()) {
    if (this->read_byte(&byte)) {
      buf_.push_back(byte);
    } else {
      break;
    }
  }
  if (!buf_.empty())
    this->process_buffer_();
}

// ... (try_parse_display_frame_, try_parse_fc_frame_, checksum, to_hex_)
}  // namespace systa_reader
}  // namespace esphome

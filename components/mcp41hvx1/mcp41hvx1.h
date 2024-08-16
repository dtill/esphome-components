
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/spi/spi.h"

namespace esphome {
namespace mcp41hvx1 {

class Mcp41hvx1Output : public output::FloatOutput,
                        public Component,
                        public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_HIGH, spi::CLOCK_PHASE_LEADING,spi::DATA_RATE_1MHZ> {
 public:
  void setup() override;
  void dump_config() override;
  void write_state(float state) override;
  void set_initial_value(float value) { this->initial_value_ = value; }
  void set_shdn_pin(GPIOPin *pin) { shdn_pin_ = pin; }
  void set_wlat_pin(GPIOPin *pin) { wlat_pin_ = pin; }

  float read_wiper();

 protected:
  void wiper_set_position(uint8_t position);
  uint8_t wiper_get_position();

  // SPI pin configuration
  uint8_t cs_pin_;
  float initial_value_;
  GPIOPin *shdn_pin_{nullptr};
  GPIOPin *wlat_pin_{nullptr};
};

}  // namespace mcp41hvx1
}  // namespace esphome
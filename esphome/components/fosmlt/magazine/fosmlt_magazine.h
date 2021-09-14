#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace fosmlt {

class Magazine : public sensor::Sensor, public Component {
 public:
  Magazine() {}

  bool has_ammo() { return this->state > 0; }
  bool has_ammo(uint8_t ammo_required) { return ((this->state - ammo_required) >= 0); }
  // can be used to increase ammo or decrease ammo
  float update_ammo(int16_t ammo_change) { this->publish_state(this->state + ammo_change); }

 protected:
  //optional GPIO to detect when a removable mag is available
  gpio::GPIOBinarySensor *magazine_available_{nullptr};

  //i2c at24c mem
  // first page is UUID of magazine, second page is max capacity, then use page_address to read/write magazine capacity
  uint16_t magazine_UUID_;
  uint16_t UUID_address_;
  
  uint16_t max_capacity_;
  uint16_t max_capacity_address_;

  uint16_t magazine_page_address_;
};

}  // namespace fosmlt
}  // namespace esphome


#pragma once

#include "esphome/components/adc/adc_sensor.h"
#include "esphome/core/log.h"
//not sure if should be included, since firemode_selector should be a child of fosmlt_tagger
#include "esphome/components/fosmlt/tagger/fosmlt_tagger.h"

/*enum FiringType {
  SAFETY = 0,
  SEMI_AUTOMATIC = 1,
  BURST_FIRE = 2,
  FULL_AUTOMATIC = 3,
  RAMPING_FIRERATE = 4,  // automatic with decreasing time between shots
  CHARGING_FIRE = 5,  // fire and release below a certain time = standard shot, else charge to a maximum amount over a
                      // set time, fires on trigger release
};*/

namespace esphome {
namespace fosmlt {

//in configuration, could accept either boolean switches for each fire mode
// eg. safety: gpio1, semi-automatic: gpio2
// or adc values
//eg safety: 0, semi-automatic: 1024 (figure out range between the two and use that as adc range)
//can test this component with a potentiometer

//also default state
//poll component every 100ms?
class FireMode {
 public:

  void set_safety(gpio::GPIOBinarySensor *safety) { safety_ = safety; }
  void set_safety(uint8_t range) { safety_range_ = range; }

 protected:
  adc::ADCSensor *rotary_ladder_;

  FiringType current_firemode_;
  uint8_t default_firemode_;

  gpio::GPIOBinarySensor *safety_{nullptr};
  uint8_t safety_range_;  //percentage

  gpio::GPIOBinarySensor *semi_automatic_{nullptr};
  gpio::GPIOBinarySensor *burst_fire_{nullptr};
  gpio::GPIOBinarySensor *full_automatic_{nullptr};
  gpio::GPIOBinarySensor *ramping_firerate_{nullptr};
  gpio::GPIOBinarySensor *charging_fire_{nullptr};
};

}  // namespace fosmlt
}  // namespace esphome

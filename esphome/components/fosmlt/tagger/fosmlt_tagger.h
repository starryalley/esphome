#pragma once

#include "esphome/core/automation.h"
#include "esphome/components/remote_transmitter/remote_transmitter.h"
#include "esphome/components/remote_base/remote_base.h"
#include "esphome/components/gpio/binary_sensor/gpio_binary_sensor.h"
#include "esphome/components/fosmlt/game/fosmlt_game.h"
#include "esphome/components/fosmlt/magazine/fosmlt_magazine.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/fosmlt/tagger/firemode_selector.h"


namespace esphome {
namespace fosmlt {

enum ReloadType {
  BUTTON_RELOAD = 0,
  RECHARGING_BATTERY = 1,
  PHYSICAL_RELOAD = 2,
};

// enum FiringType {
//   SAFETY = 0,
//   SEMI_AUTOMATIC = 1,
//   BURST_FIRE = 2,
//   FULL_AUTOMATIC = 3,
//   RAMPING_FIRERATE = 4,  // automatic with decreasing time between shots
//   CHARGING_FIRE = 5,  // fire and release below a certain time = standard shot, else charge to a maximum amount over a
//                       // set time, fires on trigger release
// };

enum CantFireReason {
  CAN_FIRE = 0,
  GAME_NOT_ACTIVE = 1,
  NOT_CONNECTED_TO_BODY = 2,
  MAGAZINE_NOT_ENOUGH_AMMO = 3,
  CHAMBERING_ROUND = 4,
  SAFETY_ON = 5,
};

enum PacketType {
  MILES = 0,
  FRAGTAG = 1,
};

class Tagger : public Component {
 public:
  Tagger() {}

  void set_trigger(gpio::GPIOBinarySensor *trigger) { trigger_ = trigger; }
  void set_reload_button(gpio::GPIOBinarySensor *reload_button) { reload_button_ = reload_button; }

  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::PROCESSOR; }

 protected:
  // setup
  remote_base::RemoteTransmitData build_shot_packet_();

  CantFireReason tagger_can_fire_();
  void fire_tagger_();
  void tagger_fired_();

  bool cockable_;  // could also include any weapon that requires cocking, etc. Can be defined alongside other reload
                   // types
  bool overheating_;  // could be used alongside other firerates, requires logic for  gradually cooling down weapon

  uint16_t damage_ = 1;
  uint16_t rounds_per_minute_ = 200;
  uint32_t time_taken_to_fire_;
  uint32_t shot_delay_ =
      (60 * 1000 /*1min to milli*/) / 200;  // convert roundsPerMinute into a millisecond delay between shots

  uint32_t last_fired_;
  // uint32_t time_can_shoot_again_;

  // player data
  char playerName[16] = "WeekendWarrior";
  uint8_t player_id_ = 170;
  uint8_t team_id_ = 2;

  gpio::GPIOBinarySensor *trigger_{nullptr};
  gpio::GPIOBinarySensor *reload_button_{nullptr};
  //fire mode selector //can be rotary switch with resistance ladder, via adc
  //or just a collection of selector switches
  //means tagger needs traits for possible fire modes?
  FiringType fire_mode_;

  bool connected_{false};

  Game *game_;
  remote_transmitter::RemoteTransmitterComponent *ir_out_{nullptr};
  remote_base::RemoteTransmitData ir_packet_;
  Magazine *magazine_;
  sensor::Sensor *tagger_battery_{nullptr};
  //Body *body_; //get status from here
  //going to want max health, max armour, max shield
  sensor::Sensor *body_battery_{nullptr};
  // sound
  //lights
  /* //display - wrapper class that declares a standard set of functions:
    Build UI
    Update UI
  */
  // solenoid
  sensor::Sensor *solenoid_battery_{nullptr};
};

// template<typename... Ts> class BrakeAction : public Action<Ts...> {
//  public:
//   explicit BrakeAction(HBridgeFan *parent) : parent_(parent) {}

//   void play(Ts... x) override { this->parent_->brake(); }

//   HBridgeFan *parent_;
// };

}  // namespace fosmlt
}  // namespace esphome

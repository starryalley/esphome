#pragma once

#include "esphome/core/automation.h"
#include "esphome/components/remote_base/remote_base.h"
#include "esphome/components/remote_receiver/remote_receiver.h"
#include "esphome/components/fosmlt/game/fosmlt_game.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace fosmlt {

class Body : public Component, public remote_base::RemoteReceiverListener {
 public:
  Body() {}

 protected:
  bool on_receive(remote_base::RemoteReceiveData data) override;

  // player data
  char playerName[16] = "WeekendWarrior";
  uint8_t player_id_ = 170;
  uint8_t team_id_ = 2;

  bool connected_{false};

  Game *game_;
  // remote_receiver::RemoteReceiverComponent *ir_in_;
  // lights

  sensor::Sensor *health_{nullptr};
  uint16_t max_health_ = 100;
  sensor::Sensor *armour_{nullptr};
  uint16_t max_armour_;
  sensor::Sensor *shield_{nullptr};
  uint16_t max_shield_;

  sensor::Sensor *body_battery_{nullptr};
  // sound
};

}  // namespace fosmlt
}  // namespace esphome

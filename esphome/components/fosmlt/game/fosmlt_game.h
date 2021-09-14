#pragma once

#include "esphome/core/automation.h"
// time

namespace esphome {
namespace fosmlt {

class Game : public Component {
 public:
  Game() {}

  // bool game_active() { return this->game_active_; }

// protected:
  bool friendly_fire_;
  bool shields_active_;
  bool armour_active_;
  bool free_for_all_;
  bool starting_peace_;
  uint32_t starting_peace_time_;
  bool game_time_limit_;
  uint32_t game_length_;
  bool stagger_enabled_;

  uint32_t timeGameStarted;  // time game started
  bool game_active_{false};

  uint8_t player_id_length_ = 8;      // Up to 256 players
  uint8_t team_id_length_ = 2;        // Up to 4 teams
  uint8_t tagger_damage_length_ = 4;  // Up to 16 different damage values that can be sent
  uint8_t packet_total_length_ = 14;

  uint8_t tagger_damage_table_[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
  uint32_t tagger_stagger_table_[16] = {100,100,100,100,100,100,100,100,100,100,100,100,100,100,0,0}; //in ms, dmgtable specific stagger times


  uint16_t ir_header_ = 2400;      // high
  uint16_t ir_1_ = 1200;           // high
  uint16_t ir_0_ = 600;            // high
  uint16_t ir_gap_ = 600;          // low
  uint16_t ir_frequency_ = 38000;  // IR freq in Hz

 protected:
};

}  // namespace fosmlt
}  // namespace esphome

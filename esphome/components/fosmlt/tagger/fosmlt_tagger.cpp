#include "fosmlt_tagger.h"
#include "esphome/core/log.h"
// #include "esphome/core/helpers.h"

namespace esphome {
namespace fosmlt {

static const char *const TAG = "fosmlt.tagger";

void Tagger::dump_config() {

}

void Tagger::setup() {

}


CantFireReason Tagger::tagger_can_fire_() {
  if (!this->game_->game_active_) {
    ESP_LOGD(TAG, "Game not active");
    return GAME_NOT_ACTIVE;
  }
  if (!this->connected_) {
    ESP_LOGD(TAG, "Tagger not connected to body");
    return NOT_CONNECTED_TO_BODY;
  }
  if (!this->magazine_->has_ammo()) {
    ESP_LOGD(TAG, "Magazine doesn't have enough ammo");
    return MAGAZINE_NOT_ENOUGH_AMMO;
  }
  if (!(millis() >= (this->last_fired_ + this->shot_delay_))) {
    // doesn't handle overflows nicely, but they only happen every 49 days
    ESP_LOGD(TAG, "Still chambering next round, can't fire yet");
    return CHAMBERING_ROUND;
  }
  if (this->fire_mode_ == SAFETY) {
    ESP_LOGD(TAG, "Tagger safety enabled");
    return SAFETY_ON;
  }

  return CAN_FIRE;
}

remote_base::RemoteTransmitData Tagger::build_shot_packet_() {
  uint32_t message = this->player_id_;
  message = (message << this->game_->team_id_length_) | this->team_id_;
  message = (message << this->game_->tagger_damage_length_) | this->damage_;
  ESP_LOGD(TAG, "Building ir shot packet:, player_id: %d, team_id: %d, tagger_dmg: %d", this->player_id_,
           this->team_id_, this->damage_);
  ESP_LOGD(TAG, "Combined ir shot packet message: %d", message);

  this->time_taken_to_fire_ = 0;

  remote_base::RemoteTransmitData data;
  // data.reset();
  data.item(this->game_->ir_header_, this->game_->ir_gap_);
  this->time_taken_to_fire_ += (this->game_->ir_header_ + this->game_->ir_gap_);

  for (uint8_t bit = 0; bit < this->game_->packet_total_length_; bit++) {
    data.mark(this->game_->ir_gap_);
    if (message & (1 << ((this->game_->packet_total_length_ - 1) - bit))) {
      data.space(this->game_->ir_1_);
      this->time_taken_to_fire_ += (this->game_->ir_1_ + this->game_->ir_gap_);
    } else {
      data.space(this->game_->ir_0_);
      this->time_taken_to_fire_ += (this->game_->ir_0_ + this->game_->ir_gap_);
    }
  }
  ESP_LOGD(TAG, "Time taken to fire ir packet %dus, (%fms)", this->time_taken_to_fire_, (this->time_taken_to_fire_ / 1000.0));

  ESP_LOGD(TAG, "Setting ir frequency to %d", this->game_->ir_frequency_);
  data.set_carrier_frequency(this->game_->ir_frequency_);
  return data;
}

void Tagger::fire_tagger_() {
  if (this->tagger_can_fire_() == CAN_FIRE) {
    ESP_LOGD(TAG, "Firing Tagger");
    auto transmit = this->ir_out_->transmit();
    auto data = transmit.get_data();
    data = &this->ir_packet_;
    transmit.perform();

    this->magazine_->update_ammo(-1);
  } else {
    ESP_LOGD(TAG, "Can't fire");
  }
}

}  // namespace fosmlt
}  // namespace esphome

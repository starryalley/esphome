#include "feedback_fan.h"
#include "esphome/core/log.h"
#include <cmath>

namespace esphome {
namespace feedback_fan {

static const char *const TAG = "stepper";

void FeedbackFan::dump_config() {
  ESP_LOGCONFIG(TAG, "FeedbackFan:");
  ESP_LOGCONFIG(TAG, "Fan '%s':", this->hbridge_fan_output_->get_name().c_str());
  ESP_LOGCONFIG(TAG, "Sensor '%s':", this->sensor_->get_name().c_str());
  // max_speed ?
  LOG_STEPPER(this);
}

void FeedbackFan::calculate_speed_(uint32_t now) {
  this->last_calculation_ = now;
  this->current_speed_ = 1e+6f / this->last_step_interval_;
  ESP_LOGD(TAG, "current_speed: %f steps/s", this->current_speed_);
}

void FeedbackFan::calculate_target_step_(float initial_velocity, uint32_t now, float accel) {
  // d=vt+((at^2)/2) transposed for t:
  float time = ((now - this->state_change_time_) / 1e+6f);  // convert micros since calc requires seconds
  this->target_step_ = initial_velocity * time + ((accel * time * time) / 2.0);
}

void FeedbackFan::turn_on_fan_(int32_t dir) {
  this->starting_position_ = this->current_position;
  auto call = this->hbridge_fan_output_->turn_on();
  // ESP_LOGD(TAG, "initial output: %d", initial_output_);
  // ESP_LOGD(TAG, "setting initial output: %d", (initial_output_ ? initial_output_ : this->hbridge_fan_output_->get_speed_count()));
  call.set_speed(initial_output_ ? initial_output_ : this->hbridge_fan_output_->get_speed_count());
  call.set_direction((fan::FanDirection) (dir < 0));
  call.perform();
  this->state_change_time_ = micros();
}

void FeedbackFan::setup() {
  this->current_position = 0;  // static_cast<int32_t>(this->sensor_->state);

  // s=ut+(at^2)/2
  float time_to_decelerate = this->max_speed_ / this->deceleration_;
  this->steps_to_decelerate_ =
      static_cast<int32_t>(this->deceleration_ * (time_to_decelerate * time_to_decelerate)) / 2;

  // runs everytime the feedback sensor changes
  this->sensor_->add_on_state_callback([this](float state) {
    this->current_position = static_cast<int32_t>(state);

    uint32_t now = micros();
    this->last_step_interval_ = (now - this->last_calculation_);
    this->calculate_speed_(now);

    // check if it's time to start decelerating
    if (this->fan_feedback_state_ != FAN_DECELERATING) {
      int32_t steps_until_end = abs(int32_t(this->target_position) - int32_t(this->current_position));
      if (steps_until_end < this->steps_to_decelerate_) {
        this->fan_feedback_state_ = FAN_DECELERATING;
        this->state_change_time_ = now;  // a bit hacky
      }
    }

    if (this->fan_feedback_state_ == FAN_DECELERATING) {
      int32_t steps_until_end = abs(int32_t(this->target_position) - int32_t(this->current_position));
      this->calculate_target_step_(this->max_speed_, now, -this->deceleration_);

      // ESP_LOGD(TAG, "Decelerating: target_steps: %f, steps: %d", this->target_step_, (this->steps_to_decelerate_ -
      // steps_until_end));
      int increment = (this->target_step_ > (this->steps_to_decelerate_ - steps_until_end)) ? 1 : -1;
      int new_speed = this->hbridge_fan_output_->speed + increment;
      this->hbridge_fan_output_->speed =
          clamp(new_speed, this->min_output_, this->hbridge_fan_output_->get_speed_count());
      this->hbridge_fan_output_->internal_update();

    } else if (this->fan_feedback_state_ == FAN_ACCELERATING) {
      // reached constant speed, stop accelerating
      if (this->current_speed_ >= this->max_speed_) {
        this->fan_feedback_state_ = FAN_MOVING;
        this->target_speed_ = this->max_speed_;
      } else {
        int32_t steps_since_start = abs(int32_t(this->current_position) - int32_t(this->starting_position_));
        this->calculate_target_step_(0, now, this->acceleration_);

        // ESP_LOGD(TAG, "Accelerating: target_steps: %f, steps: %d", this->target_step_, steps_since_start);
        if (steps_since_start != this->target_step_) {
          int increment = (this->target_step_ > steps_since_start) ? 1 : -1;
          int new_speed = this->hbridge_fan_output_->speed + increment;
          this->hbridge_fan_output_->speed =
              clamp(new_speed, this->min_output_, this->hbridge_fan_output_->get_speed_count());
          this->hbridge_fan_output_->internal_update();
        }
      }
    } else if (this->fan_feedback_state_ == FAN_MOVING) {
      // reached max speed, so track constant speed, using a simpler speed based comparison
      if (this->current_speed_ != this->target_speed_) {
        int increment = (this->current_speed_ < this->target_speed_) ? 1 : -1;
        int new_speed = this->hbridge_fan_output_->speed + increment;
        this->hbridge_fan_output_->speed =
            clamp(new_speed, this->min_output_, this->hbridge_fan_output_->get_speed_count());
        this->hbridge_fan_output_->internal_update();
      }
    }
  });
}

void FeedbackFan::loop() {
  int32_t dir = this->target_position - this->current_position;
  if (dir == 0) {
    if (this->fan_feedback_state_ == FAN_MOVING || this->fan_feedback_state_ == FAN_ACCELERATING ||
        this->fan_feedback_state_ == FAN_DECELERATING) {
      this->fan_feedback_state_ = FAN_STOPPED;
      ESP_LOGD(TAG, "FAN_MOVING -> FAN_STOPPED");
      auto call = this->hbridge_fan_output_->turn_off();
      call.perform();
    }
    return;
  }

  if (this->fan_feedback_state_ == FAN_STOPPED) {
    this->fan_feedback_state_ = FAN_ACCELERATING;
    ESP_LOGD(TAG, "FAN_STOPPED -> FAN_MOVING");
    this->turn_on_fan_(dir);
    return;
  }

  // overshot or wrong direction
  if ((fan::FanDirection) this->hbridge_fan_output_->direction != (dir < 0) &&
      (this->fan_feedback_state_ == FAN_MOVING || this->fan_feedback_state_ == FAN_ACCELERATING ||
       this->fan_feedback_state_ == FAN_DECELERATING)) {
    ESP_LOGD(TAG, "MISSED 0, changing direction");
    this->turn_on_fan_(dir);
  }
}

}  // namespace feedback_fan
}  // namespace esphome

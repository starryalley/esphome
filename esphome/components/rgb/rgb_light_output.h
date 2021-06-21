#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/light/light_output.h"

#include "esphome/core/color.h"
#include "esphome/components/light/addressable_light.h"


namespace esphome {
namespace rgb {

static const char *const TAG = "rgb";


class RGBLightOutput : public light::LightOutput {
 public:
  void set_red(output::FloatOutput *red) { red_ = red; }
  void set_green(output::FloatOutput *green) { green_ = green; }
  void set_blue(output::FloatOutput *blue) { blue_ = blue; }

  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supports_brightness(true);
    traits.set_supports_rgb(true);
    return traits;
  }


  void write_state(light::LightState *state) override {
    float red, green, blue;
    state->current_values_as_rgb(&red, &green, &blue, false);
    this->red_->set_level(red);
    this->green_->set_level(green);
    this->blue_->set_level(blue);
  }

 protected:
  output::FloatOutput *red_;
  output::FloatOutput *green_;
  output::FloatOutput *blue_;
};




class RGBLightOutputAddressable : public light::AddressableLight {
 public:
  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supports_brightness(true);
    traits.set_supports_rgb(true);
    return traits;
  }

  void setup() override {
    for (int i = 0; i < this->size(); i++) {
      //(*this)[i] = Color(0, 0, 0, 0);
      /*this->red_int_ = 0;
      this->green_int_ = 0;
      this->blue_int_ = 0;*/

      /*auto call = this->state_->make_call();
      call.set_color_temperature(this->cold_white_temperature_ +
                                 (this->warm_white_temperature_ - this->cold_white_temperature_) *
                                     (float(datapoint.value_uint) / float(this->color_temperature_max_value_)));
      call.perform();*/

      //hardwarePWMobj.write_state()  (this->red_int_, this->green_int_, this->blue_int_);
    }
  }

  int32_t size() const override { return 1; }

  void clear_effect_data() override {
    for (int i = 0; i < this->size(); i++)
      this->effect_data_[i] = 0;
  }

 protected:
  uint8_t red_int_;
  uint8_t green_int_;
  uint8_t blue_int_;
  RGBLightOutput hardwarePWMobj;
  light::LightState *state_{nullptr};
  uint8_t *effect_data_{nullptr};

};



}  // namespace rgb
}  // namespace esphome

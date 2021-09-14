#include "fosmlt_body.h"
#include "esphome/core/log.h"
// #include "esphome/core/helpers.h"

namespace esphome {
namespace fosmlt {

static const char *const TAG = "fosmlt.body";

bool Body::on_receive(remote_base::RemoteReceiveData data) {
  uint8_t message[15] = {0}; //TODO try and avoid hardcoding this array size

  // Validate header
  if (!data.expect_item(this->game_->ir_header_, this->game_->ir_gap_)) {
    return false;
  }
  return true;
}

}  // namespace fosmlt
}  // namespace esphome

#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace at24c {

// declare logging structs
// 8 bytes
struct timestampFloat {
  time_t timestamp;
  float val;
};
// 8 bytes
struct timestampInt32 {
  time_t timestamp;
  uint32_t val;
};
// 6 bytes
struct timestampInt16 {
  time_t timestamp;
  uint16_t val;
};
// 5 bytes
struct timestampInt8 {
  time_t timestamp;
  uint8_t val;
};

class AT24CComponent : public Component, public i2c::I2CDevice {
 public:
  void set_at24c_address(uint8_t address) {
    this->AT24C_ADDR_ = address;
    // do I need to do this?
    // this->set_i2c_address(this->AT24C_ADDR_);
  }
  // default table should go in python
  void set_at24c_pagesize(uint8_t pagesize) { this->AT24C_PAGESIZE_ = pagesize; }

  void setup() override;
  // void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; };

 protected:
  // I'm using AT24C256 256 Kbit, 32 KBytes, 64 bytes page, 512 pages
  uint8_t AT24C_ADDR_{0x57};
  uint8_t AT24C_PAGESIZE_{64};  // in bytes

  // optional rtc component for timestamps
  // dump function
  // easy fuction to store values should be log(val), and if the datastruct has been configured properly
  // just always work without specifying addresses

  bool at24c_readBytes(uint16_t addr, uint8_t *data, uint8_t len);
  // bool at24c_readNextBytes(uint8_t *data, uint16_t len);
  bool at24c_writeInPage(uint16_t addr, uint8 *data, uint8_t len);
  bool at24c_writeAcrossPages(uint16_t addr, uint8 *data, uint16_t len);
  bool at24c_setBytes(uint16_t addr, uint8_t val, uint16_t len);
};

template<typename... Ts> class WriteAction : public Action<Ts...>, public Parented<AT24CComponent> {
 public:
  void play(Ts... x) override { this->parent_->write_time(); }
};

template<typename... Ts> class ReadAction : public Action<Ts...>, public Parented<AT24CComponent> {
 public:
  void play(Ts... x) override { this->parent_->read_time(); }
};

template<typename... Ts> class DumpAction : public Action<Ts...>, public Parented<AT24CComponent> {
 public:
  void play(Ts... x) override { this->parent_->read_time(); }
};

}  // namespace at24c
}  // namespace esphome

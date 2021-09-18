#include "at24c.h"
#include "esphome/core/log.h"

// Datasheet:
// - https://datasheets.maximintegrated.com/en/ds/AT24C.pdf

namespace esphome {
namespace at24c {

static const char *const TAG = "at24c";

void AT24CComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up AT24C...");
  // if (!this->read_rtc_()) {
  //   this->mark_failed();
  // }
}

// void AT24CComponent::loop() { }

void AT24CComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "AT24C:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with AT24C failed!");
  }
}

float AT24CComponent::get_setup_priority() const { return setup_priority::DATA; }

// read from anywhere
// sets the address then does a normal read
// returns true to indicate success
bool AT24CComponent::at24c_readBytes(uint16_t addr, uint8_t *data, uint8_t len) {
	// set the current data address, this is the start of the write command
  // next either send the data to be written, or start a read instead
  // returns true to indicate success
	if (!this->write_byte_16(this->AT24C_ADDR_, addr)) {
    return false;
  } 
	// perform the read 
  return this->read_bytes(this->AT24C_ADDR_, data, len, 0UL);
}

// write within a page
// note if you try to write past a page boundary the write will
// wrap back to the start of the same page, so you need to know
// how much you're writing and where you're writing it to
// you don't need to start writing at the start of a page, but if you
// start in the middle you'll be able to write less before wrapping
// optionally wait for the eeprom to complete the write
// returns true to indicate success
bool AT24CComponent::at24c_writeInPage(uint16_t addr, uint8_t *data, uint8_t len) {
	// set the current data address, this is the start of the write command
  // next either send the data to be written, or start a read instead
  // returns true to indicate success
	if (!this->write_byte_16(this->AT24C_ADDR_, addr)) {
    return false;
  } 

	return this->write_bytes(this->AT24C_ADDR_, data, len);
  // bool write_bytes_raw(uint8_t address, const uint8_t *data, uint8_t len);
}

// writes across pages
// you do not need to worry about how long your data is or where you
// are writing it as it will be written in multiple parts across
// successive pages, optionally waiting for the last write to complete
// (we always have to wait for any earlier writes to complete)
// note: does not check if you are trying to write past the end of the
// eeprom!
// returns true to indicate success
bool AT24CComponent::at24c_writeAcrossPages(uint16_t addr, uint8_t *data, uint16_t len) {

	uint8_t wlen;

	// work out number of bytes available in starting page
	wlen = this->AT24C_PAGESIZE_ - (addr % this->AT24C_PAGESIZE_);
	// is that more than we actually need?
	if (wlen > len) wlen = len;

	while(wlen > 0) {
		// reduce remaining length
		len -= wlen;
		// write the page
		if (!this->at24c_writeInPage(addr, data, wlen)) {
			return false;
		}
		// advance the eeprom address and our data pointer
		addr += wlen;
		data += wlen;
		// work out how much to write next time
		wlen = (len < this->AT24C_PAGESIZE_ ? len : this->AT24C_PAGESIZE_);
	}

	return true;
}

// set an area of eeprom to specified value (like memset)
// optionally wait for completion of last write
bool AT24CComponent::at24c_setBytes(uint16_t addr, uint8_t val, uint16_t len) {

	uint8_t wlen;
	uint8_t data[this->AT24C_PAGESIZE_];

	// set the temp write buffer to user's choice of value
	os_memset(data, val, this->AT24C_PAGESIZE_);

	// work out number of bytes available in starting page
	wlen = this->AT24C_PAGESIZE_ - (addr % this->AT24C_PAGESIZE_);
	// is that more than we actually need?
	if (wlen > len) wlen = len;

	while(wlen > 0) {
		// reduce remaining length
		len -= wlen;
		// write the page
		if (!this->at24c_writeInPage(addr, data, wlen)) {
			return false;
		}
		// advance the eeprom address
		addr += wlen;
		// work out how much to write next time
		wlen = (len < this->AT24C_PAGESIZE_ ? len : this->AT24C_PAGESIZE_);
	}

	return true;
}

}  // namespace at24c
}  // namespace esphome

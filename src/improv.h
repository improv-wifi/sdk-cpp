#pragma once

#ifdef ARDUINO
#include <Arduino.h>
#endif  // ARDUINO

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

// <version> defines the library feature test macros; without it __cpp_lib_span is
// only defined once <span> itself has been included
#if defined(__has_include)
#if __has_include(<version>)
#include <version>
#endif
#endif

#if defined(__cpp_lib_span)
#include <cstring>
#include <span>
#endif  // __cpp_lib_span

namespace improv {

static const char *const SERVICE_UUID = "00467768-6228-2272-4663-277478268000";
static const char *const STATUS_UUID = "00467768-6228-2272-4663-277478268001";
static const char *const ERROR_UUID = "00467768-6228-2272-4663-277478268002";
static const char *const RPC_COMMAND_UUID = "00467768-6228-2272-4663-277478268003";
static const char *const RPC_RESULT_UUID = "00467768-6228-2272-4663-277478268004";
static const char *const CAPABILITIES_UUID = "00467768-6228-2272-4663-277478268005";

enum Error : uint8_t {
  ERROR_NONE = 0x00,
  ERROR_INVALID_RPC = 0x01,
  ERROR_UNKNOWN_RPC = 0x02,
  ERROR_UNABLE_TO_CONNECT = 0x03,
  ERROR_NOT_AUTHORIZED = 0x04,
  ERROR_BAD_HOSTNAME = 0x05,
  ERROR_UNKNOWN = 0xFF,
};

enum State : uint8_t {
  STATE_STOPPED = 0x00,
  STATE_AWAITING_AUTHORIZATION = 0x01,
  STATE_AUTHORIZED = 0x02,
  STATE_PROVISIONING = 0x03,
  STATE_PROVISIONED = 0x04,
  STATE_SCANNING = 0x05,
};

enum Command : uint8_t {
  UNKNOWN = 0x00,
  WIFI_SETTINGS = 0x01,
  IDENTIFY = 0x02,
  GET_CURRENT_STATE = 0x02,
  GET_DEVICE_INFO = 0x03,
  GET_WIFI_NETWORKS = 0x04,
  HOSTNAME = 0x05,
  DEVICE_NAME = 0x06,
  GET_NETWORK_STATE = 0x07,
  BAD_CHECKSUM = 0xFF,
};

static const uint8_t CAPABILITY_IDENTIFY = 0x01;

// Network-state flags, returned in the first byte of a GET_NETWORK_STATE (0x07) RPC response. This
// is a separate bitfield from the BLE capabilities bitmask (which reports supported RPC commands).
// NETWORK_IS_ONLINE is dynamic (current connectivity); the SUPPORTS_* bits report which interfaces
// the device has. When NETWORK_IS_ONLINE is set, reachable device URL(s) follow as later elements.
enum NetworkState : uint8_t {
  NETWORK_IS_ONLINE = 1 << 0,          // device currently has network connectivity via any interface
  NETWORK_SUPPORTS_WIFI = 1 << 1,      // Wi-Fi interface present (may be disabled)
  NETWORK_SUPPORTS_ETHERNET = 1 << 2,  // Ethernet interface present
  NETWORK_SUPPORTS_THREAD = 1 << 3,    // Thread interface present
  NETWORK_SUPPORTS_MODEM = 1 << 4,     // Cellular modem present
};
static const uint8_t IMPROV_SERIAL_VERSION = 1;

enum ImprovSerialType : uint8_t {
  TYPE_CURRENT_STATE = 0x01,
  TYPE_ERROR_STATE = 0x02,
  TYPE_RPC = 0x03,
  TYPE_RPC_RESPONSE = 0x04
};

struct ImprovCommand {
  Command command;
  std::string ssid;
  std::string password;
};

ImprovCommand parse_improv_data(const std::vector<uint8_t> &data, bool check_checksum = true);
ImprovCommand parse_improv_data(const uint8_t *data, size_t length, bool check_checksum = true);

bool parse_improv_serial_byte(size_t position, uint8_t byte, const uint8_t *buffer,
                              std::function<bool(ImprovCommand)> &&callback, std::function<void(Error)> &&on_error);

std::vector<uint8_t> build_rpc_response(Command command, const std::vector<std::string> &datum,
                                        bool add_checksum = true);
#ifdef ARDUINO
std::vector<uint8_t> build_rpc_response(Command command, const std::vector<String> &datum, bool add_checksum = true);
#endif  // ARDUINO

#if defined(__cpp_lib_span)
// Maximum encoded RPC response size: command (1) + data length (1) + string data (up to 255,
// the data length field is a single byte) + trailing checksum byte (1)
inline constexpr size_t RPC_RESPONSE_MAX_SIZE = 258;

// Builds an RPC response into a caller-provided buffer with no heap allocation.
// Produces byte-identical output to build_rpc_response(): the trailing byte is the
// checksum when requested and 0x00 otherwise.
// Defined inline so the API is fully contained in the header; a consumer compiling
// improv.cpp at a different language standard than its own sources cannot end up
// with a declared class whose methods were never emitted.
class RpcResponseBuilder {
 public:
  RpcResponseBuilder(std::span<uint8_t, RPC_RESPONSE_MAX_SIZE> buf, Command command) : buf_(buf) { buf_[0] = command; }

  // Appends one string entry (length byte + string bytes). Returns false and appends
  // nothing if the string does not fit in the response's 255 byte payload budget or
  // the response is already finished.
  bool add_string(const char *str) { return this->add_string(str, strlen(str)); }  // NUL-terminated
  bool add_string(const char *str, size_t length) {
    // The length byte and string bytes must fit in the payload budget covered by the
    // single byte data length field; the length range test also rejects out of range
    // values that could otherwise wrap the position arithmetic
    if (this->finished_ || length >= RPC_RESPONSE_MAX_SIZE || this->pos_ + 1 + length > RPC_RESPONSE_MAX_SIZE - 1)
      return false;
    this->buf_[this->pos_++] = static_cast<uint8_t>(length);
    std::memcpy(this->buf_.data() + this->pos_, str, length);
    this->pos_ += length;
    return true;
  }

  // Writes the data length and trailing byte, then returns the finished response bytes.
  // Safe to call again; later calls return the already finished response.
  std::span<const uint8_t> finish(bool add_checksum = true) {
    if (!this->finished_) {
      this->finished_ = true;
      this->buf_[1] = static_cast<uint8_t>(this->pos_ - 2);
      uint8_t trailing = 0;
      if (add_checksum) {
        uint32_t calculated_checksum = 0;
        for (size_t i = 0; i < this->pos_; i++) {
          calculated_checksum += this->buf_[i];
        }
        // Clear all bits, but the least significant byte
        trailing = static_cast<uint8_t>(calculated_checksum & 0xFF);
      }
      this->buf_[this->pos_++] = trailing;
    }
    return std::span<const uint8_t>(this->buf_.data(), this->pos_);
  }

 private:
  std::span<uint8_t, RPC_RESPONSE_MAX_SIZE> buf_;
  uint16_t pos_{2};  // max 258, fits uint16_t
  bool finished_{false};
};
#endif  // __cpp_lib_span

}  // namespace improv

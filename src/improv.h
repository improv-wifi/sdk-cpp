#pragma once

#ifdef ARDUINO
#include <Arduino.h>
#endif  // ARDUINO

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

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
  ERROR_UNKNOWN = 0xFF,
};

enum State : uint8_t {
  STATE_STOPPED = 0x00,
  STATE_AWAITING_AUTHORIZATION = 0x01,
  STATE_AUTHORIZED = 0x02,
  STATE_PROVISIONING = 0x03,
  STATE_PROVISIONED = 0x04,
};

enum Command : uint8_t {
  UNKNOWN = 0x00,
  WIFI_SETTINGS = 0x01,
  IDENTIFY = 0x02,
  GET_CURRENT_STATE = 0x02,
  GET_DEVICE_INFO = 0x03,
  GET_WIFI_NETWORKS = 0x04,
  // 0x05 (Get/Set Hostname) and 0x06 (Get/Set Device Name) are reserved by the Improv spec.
  GET_CAPABILITIES = 0x07,
  BAD_CHECKSUM = 0xFF,
};

// Device capability bitmask. Advertised over BLE (CAPABILITIES characteristic / service data) and
// returned in the first byte of a GET_CAPABILITIES serial RPC response. When CAPABILITY_ONLINE is
// set in the serial response, any reachable device URL(s) follow as subsequent string elements.
//
// CAPABILITY_ONLINE is dynamic (current connectivity); the others are static device facts. A live
// query (serial GET_CAPABILITIES) always reflects current ONLINE state; the BLE capabilities value
// is set at setup, so a device must update/notify it for ONLINE to be meaningful over BLE.
enum Capability : uint8_t {
  CAPABILITY_IDENTIFY = 0x01,  // device can identify itself (e.g. flash an LED) on request
  CAPABILITY_ONLINE = 0x02,    // device currently has network connectivity via any interface
  CAPABILITY_WIFI = 0x04,      // Wi-Fi is supported (hardware present; may be disabled)
  CAPABILITY_ETHERNET = 0x08,  // Ethernet is supported
  CAPABILITY_THREAD = 0x10,    // Thread is supported
  CAPABILITY_MODEM = 0x20,     // Cellular modem is supported
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

}  // namespace improv

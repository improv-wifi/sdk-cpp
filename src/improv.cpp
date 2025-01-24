#include "improv.h"
#include <cstring>

namespace improv {

ImprovCommand parse_improv_data(const std::vector<uint8_t> &data, bool check_checksum) {
  return parse_improv_data(data.data(), data.size(), check_checksum);
}

ImprovCommand parse_data_command(Command command, const uint8_t *data, size_t length) {
  std::vector<std::vector<uint8_t>> data_fields;
  size_t field_start = 3;
  size_t field_end = field_start + data[2];
  while (field_end < length) {
      data_fields.push_back(std::vector<uint8_t>(data + field_start, data + field_end));
      field_start = field_end + 1;
      field_end = field_start + data[field_start - 1];
  };
  return {.command = command, .data = data_fields};
}

ImprovCommand parse_improv_data(const uint8_t *data, size_t length, bool check_checksum) {
  ImprovCommand improv_command;
  Command command = (Command) data[0];
  uint8_t data_length = data[1];

  if (data_length != length - 2 - check_checksum) {
    improv_command.command = UNKNOWN;
    return improv_command;
  }

  if (check_checksum) {
    uint8_t checksum = data[length - 1];

    uint32_t calculated_checksum = 0;
    for (uint8_t i = 0; i < length - 1; i++) {
      calculated_checksum += data[i];
    }

    if ((uint8_t) calculated_checksum != checksum) {
      improv_command.command = BAD_CHECKSUM;
      return improv_command;
    }
  }

  if (command == WIFI_SETTINGS) {
    improv_command = parse_data_command(command, data, length);
    // There must be two data fields for the SSID and password
    if (improv_command.data.size() != 2) {
      improv_command.command = UNKNOWN;
    }
    return improv_command;
  }

  if (command == CUSTOM) {
    return parse_data_command(command, data, length);
  }

  improv_command.command = command;
  return improv_command;
}

bool parse_improv_serial_byte(size_t position, uint8_t byte, const uint8_t *buffer,
                              std::function<bool(ImprovCommand)> &&callback, std::function<void(Error)> &&on_error) {
  if (position == 0)
    return byte == 'I';
  if (position == 1)
    return byte == 'M';
  if (position == 2)
    return byte == 'P';
  if (position == 3)
    return byte == 'R';
  if (position == 4)
    return byte == 'O';
  if (position == 5)
    return byte == 'V';

  if (position == 6)
    return byte == IMPROV_SERIAL_VERSION;

  if (position <= 8)
    return true;

  uint8_t type = buffer[7];
  uint8_t data_len = buffer[8];

  if (position <= 8 + data_len)
    return true;

  if (position == 8 + data_len + 1) {
    uint8_t checksum = 0x00;
    for (size_t i = 0; i < position; i++)
      checksum += buffer[i];

    if (checksum != byte) {
      on_error(ERROR_INVALID_RPC);
      return false;
    }

    if (type == TYPE_RPC) {
      auto command = parse_improv_data(&buffer[9], data_len, false);
      return callback(command);
    }
  }

  return false;
}

std::vector<uint8_t> build_rpc_response(Command command, const std::vector<std::string> &datum, bool add_checksum) {
  // Calculate the byte count to reserve memory to avoid reallocations
  // Frame length fixed: 3 = Command: 1 + frame length: 1 + checksum: 1
  size_t frame_length = 3;
  // Frame length variable: string lengths: n + length of data in datum
  frame_length += datum.size();
  for (int i = 0; i < datum.size(); i++) {
    frame_length += datum[i].length();
  }
  // Reserve frame_length bytes in vector
  std::vector<uint8_t> out(frame_length, 0);

  out[0] = command;

  // Copy data from datum input to out vector with lengths
  const size_t data_offset = 2;
  size_t pos = data_offset;
  for (const auto &str : datum) {
    out[pos] = static_cast<uint8_t>(str.length());
    pos++;
    std::memcpy(out.data() + pos, str.c_str(), str.length());
    pos += str.length();
  }

  out[1] = static_cast<uint8_t>(pos - data_offset);

  if (add_checksum) {
    uint32_t calculated_checksum = 0;

    for (uint8_t byte : out) {
      calculated_checksum += byte;
    }
    // Clear all bits, but the least significant byte
    calculated_checksum &= 0xFF;
    out[frame_length - 1] = static_cast<uint8_t>(calculated_checksum);
  }
  return out;
}

#ifdef ARDUINO
std::vector<uint8_t> build_rpc_response(Command command, const std::vector<String> &datum, bool add_checksum) {
  // Calculate the byte count to reserve memory to avoid reallocations
  // Frame length fixed: 3 = Command: 1 + frame length: 1 + checksum: 1
  size_t frame_length = 3;
  // Frame length variable: string lengths: n + length of data in datum
  frame_length += datum.size();
  for (int i = 0; i < datum.size(); i++) {
    frame_length += datum[i].length();
  }
  // Reserve frame_length bytes in vector
  std::vector<uint8_t> out(frame_length, 0);

  out[0] = command;

  // Copy data from datum input to out vector with lengths
  const size_t data_offset = 2;
  size_t pos = data_offset;
  for (const auto &str : datum) {
    out[pos] = static_cast<uint8_t>(str.length());
    pos++;
    std::memcpy(out.data() + pos, str.c_str(), str.length());
    pos += str.length();
  }

  out[1] = static_cast<uint8_t>(pos - data_offset);

  if (add_checksum) {
    uint32_t calculated_checksum = 0;

    for (uint8_t byte : out) {
      calculated_checksum += byte;
    }
    // Clear all bits, but the least significant byte
    calculated_checksum &= 0xFF;
    out[frame_length - 1] = static_cast<uint8_t>(calculated_checksum);
  }
  return out;
}
#endif  // ARDUINO

}  // namespace improv

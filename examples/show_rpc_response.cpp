#include <iostream>
#include <iomanip>
#include <chrono>

#include "../src/improv.h"

int main(int argc, char *argv[]) {
  std::string msg_1 = "hi there";
  std::string msg_2 = "jojo";
  std::vector<std::string> msgs = {msg_1, msg_2};

  std::vector<uint8_t> out = improv::build_rpc_response(improv::Command::GET_DEVICE_INFO, msgs);

  auto flags = std::cout.flags();
  for (auto i : out) {
    std::cout << std::hex << std::setfill('0') << std::setw(sizeof(out[0]) * 2) << (0xFF & i) << " ";
  }
  std::cout << std::endl;
  std::cout.flags(flags);

  std::cout << "Frame len: " << out.size() << std::endl;
  std::cout << "Data len: " << (msg_1.length() + msg_2.length()) << std::endl;
}
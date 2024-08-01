#include <iostream>
#include <iomanip>
#include <chrono>

#include "../src/improv.h"

int main(int argc, char *argv[]) {
  std::string msg_1 = "hi there";
  std::string msg_2 = "jojo";
  std::vector<std::string> msgs = {msg_1, msg_2};

  const uint32_t iterations = 100000;
  printf("Running %u iterations for build_rpc_response\n", iterations);

  std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < iterations; i++) {
    std::vector<uint8_t> out = improv::build_rpc_response(improv::Command::GET_DEVICE_INFO, msgs);
  }
  std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
  // Add 1000s separator if available on system
  std::cout.imbue(std::locale(""));
  std::cout << "Total: " << (end - start).count() / 1000 << " µs" << std::endl;
  std::cout << "per run: " << (end - start).count() / iterations << " ns" << std::endl;
}
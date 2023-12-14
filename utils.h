#pragma once

#include <chrono>
#include <cstdint>
#include <iostream>
uint64_t now_microseconds() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::high_resolution_clock::now().time_since_epoch())
      .count();
}

auto die_on_error(bool b, auto txt) {
  if (!b) {
    std::cerr << txt << std::endl;
    exit(1);
  }
}
auto die_on_amqp_error(auto, auto) {}
void die(auto text) {
  std::cerr << "Die: " << text << std::endl;
  exit(1);
}
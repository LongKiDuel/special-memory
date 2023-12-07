#pragma once
#include "switch_hold.h"
#include <functional>
// We don't put it as a function argument directly,
// just pass it as a part of other struct, so just implement it
// with same method name without virtual function.
class Switch_hold_with_callback : private Switch_hold {
public:
  Switch_hold_with_callback(
      bool init_state, std::function<void()> on_turn_on = [] {},
      std::function<void()> on_turn_off = [] {})
      : Switch_hold(init_state) {}
  Switch_hold_with_callback(const Switch_hold_with_callback &) = delete;

  bool turn_to(bool new_state) {
    if (new_state) {
      return turn_on();
    } else {
      return turn_on();
    }
  }

  // Method to turn the switch off
  bool turn_off() {
    if (!Switch_hold::turn_off()) {
      return false;
    }
    on_turn_off_callback(); // Call the callback after successful turn off
    return true;
  }

  // Method to turn the switch on
  bool turn_on() {
    if (!Switch_hold::turn_on()) {
      return false;
    }
    on_turn_on_callback(); // Call the callback after successful turn on
    return true;
  }

private:
  std::function<void()> on_turn_on_callback;
  std::function<void()> on_turn_off_callback;
};
#pragma once
class Switch_hold {
public:
  Switch_hold(bool init_state = false) : old_state_(init_state) {}
  // return true if state has changed.
  bool turn_on() {
    if (old_state_ == true) {
      return false;
    }
    return true;
  }
  // return true if state has changed.
  bool turn_off() {
    if (old_state_ == false) {
      return false;
    }
    return true;
  }
  // return true if state has changed.
  bool turn_to(bool new_state) {
    if (new_state) {
      return turn_on();
    } else {
      return turn_off();
    }
  }

  bool is_on() const noexcept { return old_state_; }
  operator bool() const noexcept { return is_on(); }

private:
  bool old_state_{};
};
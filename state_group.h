#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ranges>
#include <span>

template <typename T, uint32_t max_state = 32> class State_group {
public:
  void add(T state) {
    auto pos = get_state_pos(state);
    if (pos) {
      return;
    }
    states_.at(state_count_) = state;
    state_count_++;
  }
  void remove(T state) {
    auto pos = get_state_pos(state);
    if (!pos) {
      return;
    }
    states_[*pos] = states_[state_count_ - 1];
    state_count_--;
  }
  bool has_state(T state) const { return get_state_pos(state).has_value(); }

  bool check_states(const std::ranges::input_range auto group) {
    for (auto s : group) {
      if (!has_state(s)) {
        return false;
      }
    }
    return true;
  }

private:
  std::optional<int> get_state_pos(T state) const {
    int index{};
    for (auto s : std::span<const T>{states_.data(),
                                     static_cast<size_t>(state_count_)}) {
      if (s == state) {
        return index;
      }
      index++;
    }
    return {};
  }

private:
  std::array<T, max_state> states_;
  uint32_t state_count_{};
};

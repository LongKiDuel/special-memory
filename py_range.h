#pragma once
// create range like range() in python.

template <typename T> struct Range_current_to_end {
  T current_value{};
  T end_value{};
  T operator*() const { return current_value; }
  Range_current_to_end &operator++() {
    current_value++;
    return *this;
  }
  template <typename U> bool operator!=(const U &) const {
    return current_value != end_value;
  }

  Range_current_to_end begin() const { return *this; }
  auto end() const {
    struct Empty {};
    return Empty{};
  }
};

template <typename T> Range_current_to_end<T> py_range(T end_value) {
  return {{}, end_value};
}

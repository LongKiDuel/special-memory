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
};
template <typename T>
Range_current_to_end<T> begin(Range_current_to_end<T> range) {
  return range;
}
template <typename T> auto end(const Range_current_to_end<T> &) {
  struct Empty {};
  return Empty{};
}

template <typename T> Range_current_to_end<T> py_range(T end_value) {
  return {{}, end_value};
}

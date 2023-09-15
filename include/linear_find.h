#pragma once

template <typename T, typename E>
bool linear_find(const T &container, const E &elem) {
  for (const auto &e : container) {
    if (e == elem) {
      return true;
    }
  }
  return false;
}
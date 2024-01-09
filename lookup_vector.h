#pragma once

#include <vector>
template <typename T> class Lookup_vector {
public:
  void add(T elem) { vec_.push_back(elem); }
  bool has(T elem) {
    for (auto &e : vec_) {
      if (e == elem) {
        return true;
      }
    }
    return false;
  }

private:
  std::vector<T> vec_;
};
#pragma once

#include <sstream>
#include <string>
#include <vector>

inline std::vector<std::vector<float>> import_csv(std::istream &ss) {
  std::string lineStr;
  std::vector<std::vector<float>> table;
  while (std::getline(ss, lineStr)) {
    std::stringstream lineStream{lineStr};
    std::vector<float> numbers;
    float n{};
    while (lineStream) {
      char comma;
      n = -4242189;
      auto bad = n;

      lineStream >> n >> std::ws >> comma;
      if (n != bad) {
        numbers.push_back(n);
      }
    }
    table.push_back(numbers);
  }
  return table;
}
inline std::vector<std::vector<float>> import_csv(std::string str) {
  std::stringstream ss{str};

  return import_csv(ss);
}
inline std::string export_csv(const std::vector<std::vector<float>> &table) {
  std::stringstream ss;
  for (auto &line : table) {
    for (auto n : line) {
      ss << n << " ,";
    }
    ss << "\n";
  }
  return ss.str();
}
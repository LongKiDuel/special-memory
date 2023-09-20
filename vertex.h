#pragma once

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
struct Vertex {
  glm::vec3 point{};
  glm::vec3 normal{};
  glm::vec2 uv{};

  bool operator<=>(const Vertex &) const = default;
};

#include <functional>
#include <glm/gtx/hash.hpp>
namespace std {
  template <>
  struct hash<Vertex> {
    size_t operator()(const Vertex& v) const {
      size_t hashValue = 0;

      // Combine hashes of individual components using XOR
      hashValue ^= std::hash<glm::vec3>()(v.point);
      hashValue ^= std::hash<glm::vec3>()(v.normal);
      hashValue ^= std::hash<glm::vec2>()(v.uv);

      return hashValue;
    }
  };
}
#pragma once


#include <cstdint>
#include <glm/ext/vector_float3.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vector>

struct Mesh {
  std::vector<glm::vec3> points{};
  std::vector<glm::vec3> normals{};
  std::vector<glm::vec2> uv{};

  std::vector<uint32_t> indices;
};

struct Model {
  std::vector<Mesh> meshes;
};
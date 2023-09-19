#pragma once


#include <cstdint>
#include <glm/ext/vector_float3.hpp>
#include <glm/vec3.hpp>
#include <vector>

struct Mesh {
  std::vector<glm::vec3> points;
};

struct Model {
  std::vector<Mesh> meshes;
};
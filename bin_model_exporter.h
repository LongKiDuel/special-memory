#pragma once

#include "mesh_pnu_format.h"
#include "vertex.h"
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <glm/ext/vector_float3.hpp>
#include <ios>
inline bool bin_model_exporter(Mesh_pnu_format &mesh,
                               std::filesystem::path file_name) {
  std::ofstream file{file_name, std::ios::binary};
  if (!file.is_open()) {
    return false;
  }

  auto write_number = [&file](auto number) {
    file.write(reinterpret_cast<char *>(&number), sizeof(number));
  };
  auto write_vertex = [&file](const Vertex &v) {
    file.write(reinterpret_cast<const char *>(&v), sizeof(glm::vec3) * 2);
  };

  write_number(mesh.vertices.size() * sizeof(glm::vec3) * 2);
  for (auto &v : mesh.vertices) {
    write_vertex(v);
  }
  write_number(mesh.indices.size() * sizeof(uint32_t));
  for (auto i : mesh.indices) {
    [[maybe_unused]] const auto size = mesh.vertices.size();
    assert(i < size);
    write_number(i);
  }
  return file.good();
}
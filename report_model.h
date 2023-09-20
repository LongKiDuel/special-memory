#pragma once
#include "model.h"
#include <fmt/format.h>
#include <string>

inline std::string report_mesh(const Mesh &mesh) {
  return fmt::format("Mesh has {} points {} normals {} UV {} indices",
                     mesh.points.size(), mesh.normals.size(), mesh.uv.size(),
                     mesh.indices.size());
}

inline std::string report_model(const Model &model) {
  return fmt::format("Model has {} meshes", model.meshes.size());
}

inline std::string report_model_all(const Model &model) {
  auto s = report_model(model);

  int index{};
  for (auto &mesh : model.meshes) {
    s += fmt::format("\n\tmesh {}: {}", index, report_mesh(mesh));
    index++;
  }
  return s;
}
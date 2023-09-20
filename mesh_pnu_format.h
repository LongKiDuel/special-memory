#pragma once
#include "model.h"
#include "vertex.h"
#include <cstdint>
#include <fmt/core.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
struct Mesh_pnu_format {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  Mesh_pnu_format() = default;
  Mesh_pnu_format(const Mesh &plain_buffers) {
    for (auto i : plain_buffers.indices) {
      Vertex v{};
      const bool has_uv = i < plain_buffers.uv.size();
      const bool has_normal = i < plain_buffers.normals.size();
      v.point = plain_buffers.points[i];
      if (has_normal) {
        v.normal = plain_buffers.normals[i];
      }
      if (has_uv) {
        v.uv = plain_buffers.uv[i];
      }
      vertices.push_back(v);
      indices.push_back(i);
    }
  }

  void rebuild_indices() {
    std::vector<Vertex> new_vertices;
    std::vector<uint32_t> new_indices;
    std::unordered_map<Vertex, uint32_t> vertices_pos;

    for (auto i : indices) {
      const auto &v = vertices[i];
      auto iter = vertices_pos.find(v);
      if (iter != vertices_pos.end()) {
        new_indices.push_back(iter->second);
        continue;
      }

      auto pos = new_vertices.size();
      new_vertices.push_back(v);
      new_indices.push_back(pos);
      vertices_pos[v] = pos;
    }
    vertices = std::move(new_vertices);
    indices = std::move(new_indices);
  }

  std::string report() const {
    return fmt::format("Mesh has {} vertices {} indices {} faces",
                       vertices.size(), indices.size(), indices.size() / 3);
  }
};

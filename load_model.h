#pragma once

#include "model.h"
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <complex>
#include <filesystem>
#include <optional>
#include <spdlog/spdlog.h>
#include <string>

void copy_xyz(const auto &src, auto &dest) {
  dest.x = src.x;
  dest.y = src.y;
  dest.z = src.z;
}

inline Mesh task_mesh(aiMesh *mesh) {
  Mesh m;
  for (uint64_t i{}; i < mesh->mNumVertices; i++) {
    glm::vec3 vertex{};
    const auto &v = mesh->mVertices[i];
    copy_xyz(v, vertex);
    m.points.push_back(vertex);
  }
  return m;
}

inline void task_scene_node(const aiScene *root, aiNode *node, Model &model) {
  if (!node) {
    return;
  }
  for (uint64_t i{}; i < node->mNumMeshes; i++) {
    auto mesh = root->mMeshes[node->mMeshes[i]];
    model.meshes.push_back(task_mesh(mesh));
  }
}

inline std::optional<Model> load_model(std::string path) {
  if (!std::filesystem::exists(path)) {
    SPDLOG_ERROR("failed to load model, file not exist: {}", path);
    return {};
  }
  Model m;

  Assimp::Importer importer;

  auto scene =
      importer.ReadFile(path, aiPostProcessSteps::aiProcess_Triangulate);

  if (!scene) {
    SPDLOG_ERROR("failed to load model: {}", path);
    return {};
  }

  task_scene_node(scene, scene->mRootNode, m);

  return m;
}
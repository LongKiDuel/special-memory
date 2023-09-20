#pragma once

#include "model.h"
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <complex>
#include <cstdint>
#include <filesystem>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <optional>
#include <spdlog/spdlog.h>
#include <string>

void copy_xyz(const auto &src, auto &dest) {
  dest.x = src.x;
  dest.y = src.y;
  dest.z = src.z;
}
void copy_xy(const auto &src, auto &dest) {
  dest.x = src.x;
  dest.y = src.y;
}
inline Mesh task_mesh(aiMesh *mesh) {
  Mesh m;
  auto uv_array = mesh->mTextureCoords[0];
  auto normals_array = mesh->mNormals;
  for (uint64_t i{}; i < mesh->mNumVertices; i++) {
    glm::vec3 vertex{};
    const auto &v = mesh->mVertices[i];
    copy_xyz(v, vertex);
    m.points.push_back(vertex);
    if (normals_array) {
      glm::vec3 normal{};
      copy_xyz(normals_array[i], normal);
      m.normals.push_back(normal);
    }
    if (uv_array) {
      glm::vec2 uv{};
      copy_xy(uv_array[i], uv);
      m.uv.push_back(uv);
    }
  }

  for (uint64_t i{}; i < mesh->mNumFaces; i++) {
    auto &face = mesh->mFaces[i];

    for (uint64_t j{}; j < face.mNumIndices; j++) {
      m.indices.push_back(face.mIndices[j]);
    }
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
  for (uint64_t i{}; i < node->mNumChildren; i++) {
    task_scene_node(root, node->mChildren[i], model);
  }
}

inline std::optional<Model> load_model(std::string path) {
  if (!std::filesystem::exists(path)) {
    SPDLOG_ERROR("failed to load model, file not exist: {}", path);
    return {};
  }
  Model m;

  Assimp::Importer importer;
  auto postprocess = aiPostProcessSteps::aiProcess_Triangulate |
                     aiPostProcessSteps::aiProcess_GenNormals |
                     aiPostProcessSteps::aiProcess_JoinIdenticalVertices |
                     aiPostProcessSteps::aiProcess_FixInfacingNormals |
                     aiPostProcessSteps::aiProcess_ImproveCacheLocality;
  auto scene = importer.ReadFile(path, postprocess);

  if (!scene) {
    SPDLOG_ERROR("failed to load model: {} reason: {}", path,
                 importer.GetErrorString());
    return {};
  }

  SPDLOG_INFO("model {} has {} meshes", path, scene->mNumMeshes);

  task_scene_node(scene, scene->mRootNode, m);

  return m;
}
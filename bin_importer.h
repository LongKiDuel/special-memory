#pragma once

#include "assimp/scene.h"
#include "load_model.h"
#include <assimp/BaseImporter.h>
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <cstdint>
#include <glm/ext/vector_float3.hpp>
class Bin_importer : public Assimp::BaseImporter {
  // -------------------------------------------------------------------------------
  // Returns whether the class can handle the format of the given file.
  bool CanRead(const std::string &pFile, Assimp::IOSystem *pIOHandler,
               bool checkSig) const override {
    const std::string extension = GetExtension(pFile);
    if (extension == "plain3dv2") {
      return true;
    }
    if (!extension.length() || checkSig) {
      // no extension given, or we're called a second time because no
      // suitable loader was found yet. This means, we're trying to open
      // the file and look for and hints to identify the file format.
      // #Assimp::BaseImporter provides some utilities:
      //
      // #Assimp::BaseImporter::SearchFileHeaderForToken - for text files.
      // It reads the first lines of the file and does a substring check
      // against a given list of 'magic' strings.
      //
      // #Assimp::BaseImporter::CheckMagicToken - for binary files. It goes
      // to a particular offset in the file and and compares the next words
      // against a given list of 'magic' tokens.

      // These checks MUST be done (even if !checkSig) if the file extension
      // is not exclusive to your format. For example, .xml is very common
      // and (co)used by many formats.
    }
    return false;
  }

  // -------------------------------------------------------------------------------
  // Get list of file extensions handled by this loader
  void GetExtensionList(std::set<std::string> &extensions) {
    extensions.insert("plain3dv2");
  }

  // -------------------------------------------------------------------------------
  void InternReadFile(const std::string &pFile, aiScene *pScene,
                      Assimp::IOSystem *pIOHandler) override {
    std::unique_ptr<Assimp::IOStream> file(pIOHandler->Open(pFile, "rb"));

    // Check whether we can read from the file
    if (file.get() == NULL) {
      throw DeadlyImportError("Failed to open file ", pFile, ".");
    }
    pScene->mRootNode = new aiNode{};
    // Your task: fill pScene
    // Throw a  with a meaningful (!) error message if
    // something goes wrong.
    pScene->mRootNode->mNumMeshes = 1;
    pScene->mRootNode->mMeshes = new uint32_t(0);

    pScene->mNumMeshes = 1;
    pScene->mMeshes = new aiMesh *[1];
    pScene->mMeshes[0] = new aiMesh{};
    auto mesh = pScene->mMeshes[0];
 
    auto read_number = [&file, &pFile]() {
      uint64_t n{};
      auto b = file->Read(&n, sizeof(n), 1);
      if (b != 1) {
        throw DeadlyImportError("Failed to read u64 file ", pFile, ".");
      }
      return n;
    };
    auto read_uint32 = [&file, &pFile]() {
      uint32_t n{};
      auto b = file->Read(&n, sizeof(n), 1);
      if (b != 1) {
        throw DeadlyImportError("Failed to read u32 file ", pFile, ".");
      }
      return n;
    };
    auto read_vec3 = [&file, &pFile]() {
      glm::vec3 n{};
      auto b = file->Read(&n, sizeof(n), 1);
      if (b != 1) {
        throw DeadlyImportError("Failed to read vec3 file ", pFile, ".");
      }
      return n;
    };
    auto vertices_count = read_number() / (sizeof(glm::vec3) * 2);
    mesh->mNumVertices = vertices_count;
    mesh->mNormals = (aiVector3t<float> *)new glm::vec3[vertices_count];
    mesh->mVertices = (aiVector3t<float> *)new glm::vec3[vertices_count];
    for (uint32_t i{}; i < vertices_count; i++) {
      copy_xyz(read_vec3(), mesh->mVertices[i]);
      copy_xyz(read_vec3(), mesh->mNormals[i]);
    }

    auto indices_count = read_number() / sizeof(uint32_t);
    mesh->mNumFaces = indices_count / 3;
    mesh->mFaces = new aiFace[mesh->mNumFaces];
    for (uint32_t i{}; i < mesh->mNumFaces; i++) {
      auto &face = mesh->mFaces[i];
      face.mNumIndices = 3;
      face.mIndices = new uint32_t[3];
      for (uint32_t j{}; j < 3; j++) {
        face.mIndices[j] = read_uint32();
      }
    }
  }

  const aiImporterDesc *GetInfo() const override {
    auto desc = new aiImporterDesc;
    desc->mName = "bin import";
    desc->mFileExtensions = "plain3dv2";

    return desc;
  }
};
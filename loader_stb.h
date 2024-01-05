#pragma once

#include "bitmap.h"
#include "image_loader.h"
#include <cstdio>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace image_mix {
class Loader_stb : public Loader {
public:
  std::optional<Bitmap> load(const std::string &file_path) {
    auto file_ptr = fopen(file_path.c_str(), "rb");
    if (!file_ptr) {
      // failed to open file.
      return {};
    }
    int x{};
    int y{};
    int channel{};

    auto buffer = stbi_load_from_file(file_ptr, &x, &y, &channel, 0);
    fclose(file_ptr);

    if (!buffer) {
      // failed to load file.
      return {};
    }
    Bitmap bitmap{x, y, channel};
    stbi_image_free(buffer);

    return bitmap;
  }

  virtual std::string name() const { return "stb"; }

private:
};
} // namespace image_mix
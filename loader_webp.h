#pragma once

#include "bitmap.h"
#include "image_loader.h"
#include "webp/decode.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <istream>
#include <iterator>
#include <string>
#include <vector>

namespace image_mix {
class Loader_webp : public Loader {
public:
  std::optional<Bitmap> load(const std::string &file_path) {
    std::ifstream file{file_path, std::ios::binary};
    if (!file.is_open()) {
      // failed to open file.
      return {};
    }
    std::vector<char> buffer{std::istreambuf_iterator<char>{file}, {}};

    int x{};
    int y{};
    int channel{3};

    auto data = WebPDecodeRGB(reinterpret_cast<const uint8_t *>(buffer.data()),
                              buffer.size(), &x, &y);
    if (!data) {
      // failed to parse file.
      return {};
    }

    Bitmap bitmap{x, y, channel};
    bitmap.memcpy(data);

    free(data);

    return bitmap;
  }

  virtual std::string name() const { return "webp"; }

private:
};
} // namespace image_mix
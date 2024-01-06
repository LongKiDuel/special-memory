#pragma once
#include "bitmap.h"
#include "image_exporter.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
namespace image_mix {
class Exporter_stb_jpg : public Exporter {
public:
  std::optional<std::vector<uint8_t>> export_to_buffer(const Bitmap &bitmap) {
    std::vector<uint8_t> buffer_vector{};
    auto write_back = [](void *context, void *data, int size) {
      auto buffer = reinterpret_cast<std::vector<uint8_t> *>(context);
      auto data_u8 = reinterpret_cast<uint8_t *>(data);
      buffer->insert(buffer->end(), data_u8, data_u8 + size);
    };
    stbi_write_jpg_to_func(write_back, &buffer_vector, bitmap.width(),
                           bitmap.height(), bitmap.channel(), bitmap.data(),
                           quality_factor_.value_or(70));

    return buffer_vector;
  }
  // set quality from 0 to 100.
  void set_quality(int quality) { quality_factor_ = quality; }

  std::string common_extension() const { return ".jpg"; }
  std::optional<int> quality_factor_;
};
class Exporter_stb_png : public Exporter {
public:
  std::optional<std::vector<uint8_t>> export_to_buffer(const Bitmap &bitmap) {
    std::vector<uint8_t> buffer_vector{};
    auto write_back = [](void *context, void *data, int size) {
      auto buffer = reinterpret_cast<std::vector<uint8_t> *>(context);
      auto data_u8 = reinterpret_cast<uint8_t *>(data);
      buffer->insert(buffer->end(), data_u8, data_u8 + size);
    };
    stbi_write_png_to_func(write_back, &buffer_vector, bitmap.width(),
                           bitmap.height(), bitmap.channel(), bitmap.data(),
                           bitmap.stride());

    return buffer_vector;
  }

  std::string common_extension() const { return ".png"; }
};

class Exporter_stb_bmp : public Exporter {
public:
  std::optional<std::vector<uint8_t>> export_to_buffer(const Bitmap &bitmap) {
    std::vector<uint8_t> buffer_vector{};
    auto write_back = [](void *context, void *data, int size) {
      auto buffer = reinterpret_cast<std::vector<uint8_t> *>(context);
      auto data_u8 = reinterpret_cast<uint8_t *>(data);
      buffer->insert(buffer->end(), data_u8, data_u8 + size);
    };
    stbi_write_bmp_to_func(write_back, &buffer_vector, bitmap.width(),
                           bitmap.height(), bitmap.channel(), bitmap.data());

    return buffer_vector;
  }

  std::string common_extension() const { return ".bmp"; }
};
} // namespace image_mix

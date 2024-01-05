#pragma once
#include "bitmap.h"
#include "image_exporter.h"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <webp/encode.h>
namespace image_mix {
class Exporter_webp : public Exporter {
public:
  std::optional<std::vector<uint8_t>> export_to_buffer(const Bitmap &bitmap) {
    uint8_t *buffer{};
    using Func = decltype(WebPEncodeRGB);
    auto fn = [&] {
      auto channel = bitmap.channel();
      if (channel == 3) {
        return WebPEncodeRGB;
      } else if (channel == 4) {
        return WebPEncodeRGBA;
      } else {
        assert(false);
        return WebPEncodeRGB;
      }
    }();
    size_t buffer_size =
        fn(bitmap.u8_data(), bitmap.width(), bitmap.height(), bitmap.stride(),
           quality_factor_.value_or(80), &buffer);
    if (!buffer_size) {
      return {};
    }

    std::vector<uint8_t> buffer_vector{buffer, buffer + buffer_size};
    free(buffer);
    return buffer_vector;
  }
  // set quality from 0 to 100.
  void set_quality(float quality) { quality_factor_ = quality; }

  std::string common_extension() const { return ".webp"; }
  std::optional<float> quality_factor_;
};
} // namespace image_mix

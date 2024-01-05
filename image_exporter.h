#pragma once
#include "bitmap.h"
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
namespace image_mix {
class Exporter {
public:
  virtual ~Exporter() = default;
  virtual std::optional<std::vector<uint8_t>>
  export_to_buffer(const Bitmap &bitmap) = 0;
  virtual std::string common_extension() const = 0;
};
} // namespace image_mix

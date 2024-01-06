#pragma once
#include "bitmap.h"
#include "image_exporter.h"
#include "image_exporter_stb.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
namespace image_mix {

// It store image layers in PNG format, which was introduced in Windows Vista.
class Exporter_ico : public Exporter {
public:
  // Max export size is 256 * 256.
  std::optional<std::vector<uint8_t>> export_to_buffer(const Bitmap &bitmap) {
    if (bitmap.width() > 256 || bitmap.height() > 256) {
      // image is to big!
      return {};
    }
    Exporter_stb_png exporter;
    auto layer = exporter.export_to_buffer(bitmap);
    if (!layer) {
      // failed to encode file.
      return {};
    }
    reset(); // clear old buffers.

    sizes_.push_back({static_cast<uint8_t>(bitmap.width()),
                      static_cast<uint8_t>(bitmap.height())});
    buffers_.push_back(std::move(*layer));

    return export_to_buffer();
  }

  std::string common_extension() const { return ".ico"; }

private:
  void reset() { buffers_.clear(); }
  std::optional<std::vector<uint8_t>> export_to_buffer() {
    auto buffer_size = compute_buffer_size();
    std::vector<uint8_t> buffer;
    buffer.resize(buffer_size);
    Icon_dir header{};
    header.image_count_ = 1;
    std::vector<Icon_dir_entry> entries{};
    uint32_t cursor = compute_header_size();
    for (uint32_t i{}; i < buffers_.size(); i++) {
      Icon_dir_entry entry{};
      entry.image_buffer_size_ = buffers_[i].size();
      entry.size_ = sizes_[i];
      entry.use_color_palette_ = false;
      // Maybe this value is wrong.
      entry.bits_per_pixel_or_pixel_to_top_ = 8;
      entry.absolute_offset_in_file_ = cursor;
      cursor += entry.image_buffer_size_;
      entries.push_back(entry);
    }

    cursor = 0;
    auto insert = [&cursor, &buffer](auto *pt, size_t bytes_count) {
      assert(cursor + bytes_count <= buffer.size());
      memcpy(buffer.data() + cursor, pt, bytes_count);
      cursor += bytes_count;
    };

    insert(&header, sizeof(header));
    for (auto &e : entries) {
      insert(&e, sizeof(e));
    }
    for (auto &img : buffers_) {
      insert(img.data(), img.size());
    }
    assert(cursor == buffer_size);

    return buffer;
  }
  struct Image_size {
    uint8_t width_;
    uint8_t height_;
  };

  struct Icon_dir {
    const uint16_t reserved_{};
    const uint16_t image_type{1}; // 1 or 2, 1 for ICO, 2for CUR
    uint16_t image_count_{};
  };
  struct Icon_dir_entry {
    Image_size size_;
    bool use_color_palette_;
    const uint8_t reserved_{};
    uint16_t dynamic_;
    uint16_t bits_per_pixel_or_pixel_to_top_;
    uint32_t image_buffer_size_;
    uint32_t absolute_offset_in_file_;
  };
  size_t compute_header_size() {
    static_assert(sizeof(Icon_dir) == 6);
    static_assert(sizeof(Icon_dir_entry) == 16);
    return sizeof(Icon_dir) + sizeof(Icon_dir_entry) * buffers_.size();
  }
  size_t compute_buffer_size() {
    auto size = compute_header_size();
    for (auto &b : buffers_) {
      size += b.size();
    }
    return size;
  }

  std::vector<Image_size> sizes_;
  std::vector<std::vector<uint8_t>> buffers_;
};
} // namespace image_mix

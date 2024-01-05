#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
namespace image_mix {
struct Bitmap_info {
  int width_;
  int height_;
  int channel_;
  int bits_;
};
class Bitmap {
public:
  Bitmap(int width, int height, int channel = 4, int bits = 8) {
    buffer_.resize(get_buffer_size(width, height, channel, bits));
    info_ = {width, height, channel, bits};
  }

  Bitmap_info info_;
  char *data() { return buffer_.data(); }
  const char *data() const { return buffer_.data(); }
  const uint8_t *u8_data() const {
    return reinterpret_cast<const uint8_t *>(data());
  }
  size_t size() const noexcept { return buffer_.size(); }
  size_t stride() const {
    return get_row_size(width(), channel(), info_.bits_);
  }

  int width() const { return info_.width_; }
  int height() const { return info_.height_; }
  int channel() const { return info_.channel_; }

  // copy from same sized buffer.
  void memcpy(void *data) { ::memcpy(buffer_.data(), data, buffer_.size()); }

private:
  static std::size_t get_row_size(int width, int channel, int bits) {
    assert(bits == 8);
    return width * channel;
  }
  static std::size_t get_buffer_size(int width, int height, int channel,
                                     int bits) {
    return get_row_size(width, channel, bits) * height;
  }

  std::vector<char> buffer_;
};
} // namespace image_mix
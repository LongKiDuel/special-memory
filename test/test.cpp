#include <algorithm>
#include <gtest/gtest.h>

struct Rect_fit_result {
  int item_width_{};
  int item_height_{};
  int item_offset_width_{};
  int item_offset_height_{};
  float scale_{};
};
constexpr Rect_fit_result compute_rect_fit(int container_width,
                                           int container_height, int width,
                                           int height) {
  float width_scale =
      static_cast<float>(container_width) / static_cast<float>(width);
  float height_scale =
      static_cast<float>(container_height) / static_cast<float>(height);
  float min_scale = std::min(width_scale, height_scale);
  Rect_fit_result result{};
  result.item_width_ = min_scale * width;
  result.item_height_ = min_scale * height;
  result.item_offset_width_ = (container_width - result.item_width_) / 2;
  result.item_offset_height_ = (container_height - result.item_height_) / 2;
  result.scale_ = min_scale;
  return result;
}
// Demonstrate some basic assertions.
TEST(computeRectTest, BasicAssertions) {
  const int device_width = 640;
  const int device_height = 1280;

  {
    const int width = 320;
    const int height = 1001;
    constexpr auto result =
        compute_rect_fit(device_width, device_height, width, height);
    EXPECT_EQ(result.scale_, static_cast<float>(device_height) / height);
    EXPECT_EQ(device_height, height * result.scale_);
    EXPECT_EQ(result.item_height_, device_height);
  }

  {
    const int width = 320;
    const int height = 500;
    constexpr auto result =
        compute_rect_fit(device_width, device_height, width, height);
    EXPECT_EQ(result.item_width_, device_width);
  }

  {
    const int width = 1320;
    const int height = 500;
    constexpr auto result =
        compute_rect_fit(device_width, device_height, width, height);
    EXPECT_EQ(result.item_width_, device_width);
  }
  {
    const int width = 1320;
    const int height = 5000;
    constexpr auto result =
        compute_rect_fit(device_width, device_height, width, height);
    EXPECT_EQ(result.item_height_, device_height);
  }
}

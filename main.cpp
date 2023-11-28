#include <glm/fwd.hpp>
#include <glm/gtx/color_space.hpp>
#include <iostream>

class Hue_ring {
public:
  Hue_ring(glm::vec3 base_hsv = {0, 0.7, 0.7}, float hue_step = 15)
      : base_(base_hsv), current_(base_), step_(hue_step) {}
  glm::vec3 next_rgb() {
    next();
    return glm::rgbColor(current_);
  }
  float step() const { return step_; }

private:
  void next() {
    current_.x += step_;
    if (current_.x >= 360) {
      current_.x -= 360;
    }
  }
  glm::vec3 base_;
  glm::vec3 current_;

  float step_;
};
#include <fmt/core.h>
#include <glm/glm.hpp>

// Custom formatter for glm::vec3
template <> struct fmt::formatter<glm::vec3> {
  // Parses format specifications of the form ['f' | 'e']
  constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
    // [ctx.begin(), ctx.end()) is a character range that contains a part of
    // the format specification (excluding braces).
    auto it = ctx.begin(), end = ctx.end();

    // Check if reached the end of the range:
    if (it != end && (*it == 'f' || *it == 'e')) {
      // Advance a character, as we've found a valid format specifier
      it++;
    }

    // Return an iterator past the end of the parsed range:
    return it;
  }

  // Formats the point p using the parsed format specification (either 'f' or
  // 'e').
  template <typename FormatContext>
  auto format(const glm::vec3 &vec, FormatContext &ctx) -> decltype(ctx.out()) {
    // ctx.out() is an output iterator to write to.
    return format_to(ctx.out(), "[{:.2f}, {:.2f}, {:.2f}]", vec.x, vec.y,
                     vec.z);
  }
};

int main() {
  Hue_ring ring{};

  for (int i = 0; i < (360 / ring.step()); i++) {
    std::cout << fmt::format("color {}: {}\n", i, ring.next_rgb());
  }
  for (int i = 0; i < (360 / ring.step()); i++) {
    auto color = ring.next_rgb();
    std::cout << fmt::format("<p style=\"color:rgb({},{},{});\">This is a "
                             "paragraph with {} color.</p>",
                             color.r * 255.99, color.g * 255.99,
                             color.b * 255.99, color);
  }
}

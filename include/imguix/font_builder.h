#pragma once

#include <cstdint>
// #include <fmt/format.h>
#include <imgui.h>
#include <set>
#include <string_view>
#include <vector>
namespace ImGuiX {
class Font_builder {
public:
  void add(std::string_view text) {
    builder_.AddText(text.begin(), text.end());
    // auto data = text.data();
    // uint32_t character{};
    // while (data < text.end()) {
    //   int error{};
    //   data = fmt::detail::utf8_decode(data, &character, &error);
    //   fonts_.insert(character);
    //   if (error) {
    //     break;
    //   }
    // }
  }
  void add(const ImWchar *ranges) { builder_.AddRanges(ranges); }
  void add(uint32_t character) { builder_.AddChar(character); }

  std::vector<ImWchar> get_range() {
    ImVector<ImWchar> ranges;
    builder_.BuildRanges(&ranges);
    return {ranges.begin(), ranges.end()};
  }

private:
  ImFontGlyphRangesBuilder builder_{};
};
} // namespace ImGuiX
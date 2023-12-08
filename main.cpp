#include <cstdint>
#include <iostream>
#include <optional>
#include <string_view>
#define IMGUI_DEFINE_MATH_OPERATORS
///
#include "imguix/window.h"
#include "include/imguix/app.h"
#include <cmath>
#include <functional>
#include <imgui.h>
#include <memory>
#include <string>
#include <vector>

#include "window_slot.h"

enum class App_state {
  loadding,
  running,
  pause,
  stopped,

  debug_window,
};

// get a input, then select best matched text from a group of resoucre.
//
// e.g.
// input: bultd
// src: build, back, help
// return [build, back, help]
namespace text_rating {

struct Rating_result {
  bool is_sub_str{};
  int same_character_out_of_order_count{};
  int in_order_sub_str_count{};
};

Rating_result rate_the_text(std::string_view pattern_text,
                            std::string_view text_to_rate) {
  Rating_result result;

  // Check if pattern_text is a substring of text_to_rate
  result.is_sub_str = text_to_rate.find(pattern_text) != std::string_view::npos;

  // Count same characters, regardless of their order
  for (char c : pattern_text) {
    if (text_to_rate.find(c) != std::string_view::npos) {
      result.same_character_out_of_order_count++;
    }
  }

  // Count longest in-order substring
  size_t start = 0;
  while ((start = text_to_rate.find(pattern_text[0], start)) !=
         std::string_view::npos) {
    int count = 0;
    size_t pattern_idx = 0;
    for (size_t i = start;
         i < text_to_rate.size() && pattern_idx < pattern_text.size(); ++i) {
      if (text_to_rate[i] == pattern_text[pattern_idx]) {
        count++;
        pattern_idx++;
      }
    }
    result.in_order_sub_str_count =
        std::max(result.in_order_sub_str_count, count);
    start++;
  }

  return result;
}

int64_t get_score(Rating_result result) {
  int64_t score = 0;
  score +=
      result.is_sub_str ? 1000 : 0; // Arbitrary high value for substring match
  score += result.same_character_out_of_order_count *
           10; // Each out-of-order match adds 10
  score +=
      result.in_order_sub_str_count * 100; // Each in-order character adds 100
  return score;
}

std::vector<std::string_view>
rate_texts(std::string_view pattern, const std::vector<std::string_view> &texts,
           int limit = 10) {
  if (pattern.empty()) {
    return {};
  }
  std::vector<std::pair<std::string_view, int64_t>> scored_texts;

  for (auto &text : texts) {
    auto result = rate_the_text(pattern, text);
    auto score = get_score(result);
    if (score > 0) {
      scored_texts.emplace_back(text, score);
    }
  }

  std::sort(scored_texts.begin(), scored_texts.end(),
            [](const auto &a, const auto &b) { return a.second > b.second; });

  std::vector<std::string_view> top_texts;
  for (int i = 0; i < std::min(limit, static_cast<int>(scored_texts.size()));
       ++i) {
    top_texts.push_back(scored_texts[i].first);
  }

  return top_texts;
}
} // namespace text_rating

#include <imgui_stdlib.h>
void wrapping_test() {
  bool is_debug = false;
  static std::string input;
  std::vector<std::string_view> command_group = {"build", "help", "clean"};

  ImGui::InputText("input", &input);

  auto result = text_rating::rate_texts(input, command_group);

  for (auto t : result) {
    ImGui::Text("%s", t.data());
  }
}
int main() {
  auto app = ImGuiX::create_vulkan_app();

  app->add_window(std::make_shared<Window_slot>("Wrapping", wrapping_test));

  app->run();
}

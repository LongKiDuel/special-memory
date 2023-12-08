#include <cstdint>
#include <cstdlib>
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

  int length_diff{};
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
  result.length_diff = text_to_rate.size() - pattern_text.size();

  return result;
}

float get_score(Rating_result result) {
  float score = 0;
  score +=
      result.is_sub_str ? 1000 : 0; // Arbitrary high value for substring match
  score += result.same_character_out_of_order_count *
           10; // Each out-of-order match adds 10
  score +=
      result.in_order_sub_str_count * 100; // Each in-order character adds 100

  if (result.length_diff > 0) {
    score -= std::abs(result.length_diff);
  } else {
    score -=
        std::abs(result.length_diff) * 0.5; // less cost for shorter string.
  }
  return score;
}

struct Filter_config {
  std::optional<float> max_distance_from_1st{};
  std::optional<int> limit{10};
};
std::vector<std::string_view>
rate_texts(std::string_view pattern, const std::vector<std::string_view> &texts,
           Filter_config config = {}) {
  if (pattern.empty()) {
    return {};
  }
  std::vector<std::pair<std::string_view, float>> scored_texts;

  for (auto &text : texts) {
    auto result = rate_the_text(pattern, text);
    auto score = get_score(result);
    if (score > 0) {
      scored_texts.emplace_back(text, score);
    }
  }
  if (scored_texts.empty()) {
    return {};
  }
  std::sort(scored_texts.begin(), scored_texts.end(),
            [](const auto &a, const auto &b) { return a.second > b.second; });

  auto max_score = scored_texts[0].second;

  std::vector<std::string_view> top_texts;
  for (int i = 0; i < std::min(config.limit.value_or(999999),
                               static_cast<int>(scored_texts.size()));
       ++i) {
    if (config.max_distance_from_1st) {
      if (max_score - scored_texts[i].second >
          config.max_distance_from_1st.value()) {
        continue;
      }
    }
    top_texts.push_back(scored_texts[i].first);
  }

  return top_texts;
}
} // namespace text_rating

#include <imgui_stdlib.h>
std::vector<std::string_view> command_group;

void wrapping_test() {
  bool is_debug = false;
  static std::string input;

  ImGui::InputText("input", &input);

  text_rating::Filter_config config{};
  config.max_distance_from_1st = 300;
  auto result = text_rating::rate_texts(input, command_group, config);

  for (auto t : result) {
    ImGui::Text("%s", t.data());
  }
}

#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

std::string remove_punctuation_with_space(const std::string &word) {
  std::string result;
  for (char ch : word) {
    if (!std::ispunct(static_cast<unsigned char>(ch))) {
      result += ch;
    } else {
      result += ' ';
    }
  }
  return result;
}

std::vector<std::string> load_unique_words(const std::string &file_name) {
  std::ifstream file(file_name);
  if (!file.is_open()) {
    throw std::runtime_error("Unable to open file: " + file_name);
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string content = remove_punctuation_with_space(buffer.str());

  std::istringstream iss(content);
  std::vector<std::string> words;
  std::set<std::string> unique_words;
  std::string word;

  while (iss >> word) {
    if (unique_words.find(word) == unique_words.end()) {
      unique_words.insert(word);
      words.push_back(word);
    }
  }

  return words;
}

int main(int argc, char **argv) {
  auto app = ImGuiX::create_vulkan_app();
  std::vector<std::string> words;

  if (argc > 1) {
    words = load_unique_words(argv[1]);
  } else {
    words = {"build", "help", "clean"};
  }

  for (auto &w : words) {
    command_group.push_back(w);
  }

  app->add_window(std::make_shared<Window_slot>("Wrapping", wrapping_test));

  app->run();
}

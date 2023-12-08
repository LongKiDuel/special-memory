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
struct Rating_result {};
Rating_result rate_the_text(std::string_view pattern_text,
                            std::string_view text_to_rate) {}
int64_t get_socer(Rating_result result) {}
std::vector<std::string_view> rate_texts(std::string_view pattern,
                                         std::vector<std::string_view> texts,
                                         int limit = 10) {}
} // namespace text_rating
void wrapping_test() {
  bool is_debug = false;
  static std::string input;
  std::vector<std::string> command_group = {"build", "help", "clean"};

  ImGui::Text("%s", is_debug ? "debug" : "no debug");
}
int main() {
  auto app = ImGuiX::create_vulkan_app();

  app->add_window(std::make_shared<Window_slot>("Wrapping", wrapping_test));

  app->run();
}

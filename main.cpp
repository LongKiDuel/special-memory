#include "state_group.h"
#include <cstdint>
#include <optional>
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

#include "switch_hold_with_callback.h"
#include "window_slot.h"

enum class App_state {
  loadding,
  running,
  pause,
  stopped,

  debug_window,
};

State_group<App_state> app_states;
void wrapping_test() {
  bool is_debug = false;
  if (app_states.check_states(std::array{App_state::debug_window})) {
    is_debug = true;
  }
  if (ImGui::Button("add debug")) {
    app_states.add(App_state::debug_window);
  }
  if (ImGui::Button("remove debug")) {
    app_states.remove(App_state::debug_window);
  }
  ImGui::Text("%s", is_debug ? "debug" : "no debug");
}
int main() {
  auto app = ImGuiX::create_vulkan_app();

  app->add_window(std::make_shared<Window_slot>("Wrapping", wrapping_test));

  app->run();
}

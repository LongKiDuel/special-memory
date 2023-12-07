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

void wrapping_test() {}
int main() {
  auto app = ImGuiX::create_vulkan_app();

  app->add_window(std::make_shared<Window_slot>("Wrapping", wrapping_test));

  app->run();
}

#include "date_range.h"
#include "imguix/window_slot.h"
#include "item_select.h"
#include <optional>
#define IMGUI_DEFINE_MATH_OPERATORS
///

#include "include/imguix/app.h"
#include <cmath>
#include <functional>
#include <imgui.h>
#include <memory>
#include <string>
#include <vector>

void paint() {

  // Usage example in your main ImGui loop
  static float timeValue = 1.0f;      // Initial time value
  static TimeUnit timeUnit = Seconds; // Initial time unit
  TimeInputWidget("Time Input", &timeValue, &timeUnit);
}
int main() {
  auto app = ImGuiX::create_vulkan_app();

  // app->add_window(std::make_shared<Window_slot>("Table", table));
  app->add_window(std::make_shared<ImGuiX::Window_slot>("Draw", paint));
  app->add_window(std::make_shared<ImGuiX::Window_slot>("Date", render_date));

  app->run();
}

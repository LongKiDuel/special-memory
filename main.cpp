#include <optional>
#define IMGUI_DEFINE_MATH_OPERATORS
///
#include "imguix/window.h"
#include "include/imguix/app.h"
#include <cmath>
#include <functional>
#include <imgui.h>
#include <implot.h>
#include <memory>
#include <string>
#include <vector>
class Window_slot : public ImGuiX::Window {
public:
  Window_slot(std::string title, std::function<void()> call)
      : ImGuiX::Window(title), draw_call_(call) {}
  void draw_content() { draw_call_(); }
  std::function<void()> draw_call_;
};

void paint() {
  std::function<float(float)> func = [](float x) { return x; };
  if (ImPlot::BeginPlot("My Plot")) {
    std::vector<float> x_axis{};
    std::vector<float> y_axis{};
    const int count = 1000;
    for (int i = 0; i <= count; i++) {
      x_axis.push_back(i * 1.f / (count));
      y_axis.push_back(func(x_axis.back()));
    }
    ImPlot::PlotLine("My Line Plot", x_axis.data(), y_axis.data(), count);

    ImPlot::EndPlot();
  }
}
int main() {
  auto app = ImGuiX::create_vulkan_app();

  ImPlot::CreateContext();
  // app->add_window(std::make_shared<Window_slot>("Table", table));
  app->add_window(std::make_shared<Window_slot>("Draw", paint));

  app->run();
}

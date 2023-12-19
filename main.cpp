#include <cstdint>
#include <map>
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
void draw_movement(float ratio) {
  auto start = ImGui::GetWindowPos();
  auto width = ImGui::GetWindowWidth();
  auto height = ImGui::GetWindowHeight();
  auto draw = ImGui::GetWindowDrawList();

  float h_ratio = 0.7;
  start.y += h_ratio * height;

  const float radius = 4;
  draw->AddCircleFilled({start.x + width * ratio, start.y}, radius, -1);
}
void paint() {
  std::map<std::string, std::function<float(float)>> functions = {
      {"linear", [](float x) { return x; }},
      {"square", [](float x) { return x * x; }},
      {"cubic", [](float x) { return x * x * x; }},
      {"square root", [](float x) { return std::sqrt(x); }},
  };

  if (ImGui::BeginTabBar("Functions")) {
    for (auto [name, func] : functions) {
      if (ImGui::BeginTabItem(name.c_str())) {
        if (ImPlot::BeginPlot("My Plot")) {
          std::vector<float> x_axis{};
          std::vector<float> y_axis{};
          const int count = 1000;
          for (int i = 0; i <= count; i++) {
            x_axis.push_back(i * 1.f / (count));
            y_axis.push_back(func(x_axis.back()));
          }
          ImPlot::PlotLine("My Line Plot", x_axis.data(), y_axis.data(), count);

          {
            auto t = ImGui::GetTime();
            uint64_t index = t * 1000;
            index %= count;
            auto ratio = y_axis[index];
            draw_movement(ratio);
          }

          ImPlot::EndPlot();
        }
        ImGui::EndTabItem();
      }
    }
    ImGui::EndTabBar();
  }
}
int main() {
  auto app = ImGuiX::create_vulkan_app();

  ImPlot::CreateContext();
  // app->add_window(std::make_shared<Window_slot>("Table", table));
  app->add_window(std::make_shared<Window_slot>("Draw", paint));

  app->run();
}

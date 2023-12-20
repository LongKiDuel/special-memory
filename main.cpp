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
class Window_slot : public ImGuiX::Window {
public:
  Window_slot(std::string title, std::function<void()> call)
      : ImGuiX::Window(title), draw_call_(call) {}
  void draw_content() { draw_call_(); }
  std::function<void()> draw_call_;
};
class Window_overlay : public ImGuiX::Window {
public:
  Window_overlay(std::string title, std::function<void()> call)
      : ImGuiX::Window(title), draw_call_(call) {}
  bool window_begin() override {
    // overlay from imgui.
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    if (location >= 0) {
      const float PAD = 10.0f;
      const ImGuiViewport *viewport = ImGui::GetMainViewport();
      ImVec2 work_pos =
          viewport
              ->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
      ImVec2 work_size = viewport->WorkSize;
      ImVec2 window_pos, window_pos_pivot;
      window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD)
                                    : (work_pos.x + PAD);
      window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD)
                                    : (work_pos.y + PAD);
      window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
      window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
      ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
      ImGui::SetNextWindowViewport(viewport->ID);
      window_flags |= ImGuiWindowFlags_NoMove;
    } else if (location == -2) {
      // Center window
      ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(),
                              ImGuiCond_Always, ImVec2(0.5f, 0.5f));
      window_flags |= ImGuiWindowFlags_NoMove;
    }
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    return open_ &&
           ImGui::Begin("Example: Simple overlay", &open_, window_flags);
  }
  void window_end() override {
    if (!closed_) {
      ImGui::End();
      if (!open_) {
        closed_ = true;
      }
    }
  }
  void draw_content() override {
    auto p_open = &open_;
    ImGui::Text("Simple overlay\n"
                "(right-click to change position)");
    ImGui::Separator();
    ImGuiIO &io = ImGui::GetIO();
    if (ImGui::IsMousePosValid())
      ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
    else
      ImGui::Text("Mouse Position: <invalid>");
    if (ImGui::BeginPopupContextWindow()) {
      if (ImGui::MenuItem("Custom", NULL, location == -1))
        location = -1;
      if (ImGui::MenuItem("Center", NULL, location == -2))
        location = -2;
      if (ImGui::MenuItem("Top-left", NULL, location == 0))
        location = 0;
      if (ImGui::MenuItem("Top-right", NULL, location == 1))
        location = 1;
      if (ImGui::MenuItem("Bottom-left", NULL, location == 2))
        location = 2;
      if (ImGui::MenuItem("Bottom-right", NULL, location == 3))
        location = 3;
      if (p_open && ImGui::MenuItem("Close"))
        *p_open = false;
      ImGui::EndPopup();
    }
    draw_call_();
  }
  std::function<void()> draw_call_;
  bool open_{true};
  bool closed_{false};
  int location{};
};

void paint() {
  if (ImGui::Button("maxmize")) {
    if (!ImGui::IsWindowDocked()) {
      auto dock_id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
      ImGui::SetNextWindowDockID(dock_id);
    }
  }
}
int main() {
  auto app = ImGuiX::create_vulkan_app();

  // app->add_window(std::make_shared<Window_slot>("Table", table));
  app->add_window(std::make_shared<Window_slot>("Draw", paint));
  app->add_window(std::make_shared<Window_overlay>("Draw", paint));

  app->run();
}

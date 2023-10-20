#include "imguix/vulkan_app.h"
#include <imguix/app.h>
#include <imguix/window.h>
#include <memory>
void ImGuiX::App::draw() {
  for (auto &w : windows) {
    w->run();
  }
}
ImGuiX::App::~App() {}
ImGuiX::App::App() {}
void ImGuiX::App::frame_begin() {}
void ImGuiX::App::frame_end() {}
bool ImGuiX::App::should_stop() { return false; }
void ImGuiX::App::run() {
  while (!should_stop()) {
    frame_begin();
    draw();
    frame_end();
  }
}
void ImGuiX::App::add_window(std::shared_ptr<Window> window) {
  windows.push_back(std::move(window));
}
void ImGuiX::App::remove_window(std::shared_ptr<Window> window) {
  std::erase(windows, window);
}
namespace ImGuiX {

std::shared_ptr<App> create_vulkan_app() {
  return std::make_shared<Vulkan_app>();
}
} // namespace ImGuiX

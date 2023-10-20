#include "imguix/window.h"
#include "include/imguix/app.h"
#include <functional>
#include <memory>
#include <string>
class WindowSlot : public ImGuiX::Window {
public:
  WindowSlot(std::string title, std::function<void()> call)
      : ImGuiX::Window(title), draw_call_(call) {}
  void draw_content() { draw_call_(); }
  std::function<void()> draw_call_;
};
int main() {
  auto app = ImGuiX::create_vulkan_app();

  auto call = []() {

  };
  app->add_window(std::make_shared<WindowSlot>("Table",call));

  app->run();
}

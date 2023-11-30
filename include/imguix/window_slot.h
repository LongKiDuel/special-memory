#pragma once
#include "imguix/window.h"
#include <functional>
#include <string>
namespace ImGuiX {

class Window_slot : public Window {
public:
  Window_slot(std::string title, std::function<void()> call)
      : ImGuiX::Window(title), draw_call_(call) {}
  void draw_content() { draw_call_(); }
  std::function<void()> draw_call_;
};

} // namespace ImGuiX
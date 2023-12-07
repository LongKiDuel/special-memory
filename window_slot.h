#pragma once
#include "imguix/window.h"
#include "include/imguix/app.h"
#include <functional>
#include <string>
class Window_slot : public ImGuiX::Window {
public:
  Window_slot(std::string title, std::function<void()> call)
      : ImGuiX::Window(title), draw_call_(call) {}
  void draw_content() { draw_call_(); }
  std::function<void()> draw_call_;
};

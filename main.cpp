#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
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
#include <tracy/Tracy.hpp>
#include <vector>
class Window_slot : public ImGuiX::Window {
public:
  Window_slot(std::string title, std::function<void()> call)
      : ImGuiX::Window(title), draw_call_(call) {}
  void draw_content() { draw_call_(); }
  std::function<void()> draw_call_;
};

void paint() { ZoneScoped; }
int main() {
#ifdef TRACY_ENABLE
  std::cout << "TRACY: " << TRACY_ENABLE << "\n";
#endif
  auto app = ImGuiX::create_vulkan_app();

  // app->add_window(std::make_shared<Window_slot>("Table", table));
  app->add_window(std::make_shared<Window_slot>("Draw", paint));

  app->run();
}

void *operator new(size_t size) {
  auto pt = malloc(size);
  TracyAlloc(pt, size);
  // printf("%p %zu\n", pt, size);
  return pt;
}
void operator delete(void *pt) noexcept {
  // printf("%p od\n", pt);
  TracyFree(pt);
  free(pt);
}
void operator delete(void *pt, size_t size) noexcept {
  // printf("%p d\n", pt);
  TracyFree(pt);
  free(pt);
}
#pragma once

#include <memory>
#include <vector>

namespace ImGuiX {
class Window;
class App {
public:
  App();
  virtual ~App();
  virtual void draw();
  virtual void frame_begin();
  virtual void frame_end();
  virtual bool should_stop();
  void run();

  void add_window(std::shared_ptr<Window> window);

  void remove_window(std::shared_ptr<Window> window);

private:
  std::vector<std::shared_ptr<Window>> windows;
};

std::shared_ptr<App> create_vulkan_app();
} // namespace ImGuiX
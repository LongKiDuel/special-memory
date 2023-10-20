#pragma once
#include <string>
#include <string_view>
namespace ImGuiX {

class Window {
public:
  Window(std::string title) : title_(title) {}
  virtual ~Window() = default;
  virtual bool window_begin();
  virtual void window_end();
  virtual void draw_content();
  virtual void run();

  void set_title(std::string s);
  std::string_view get_title() const;

private:
  std::string title_;
};
} // namespace ImGuiX
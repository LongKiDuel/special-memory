#include <imguix/window.h>
#include <imgui.h>
bool ImGuiX::Window::window_begin() { return ImGui::Begin(title_.c_str()); }
void ImGuiX::Window::window_end() { ImGui::End(); }
void ImGuiX::Window::run() {
  if (window_begin()) {
    draw_content();
  }
  window_end();
}
void ImGuiX::Window::draw_content() {}
void ImGuiX::Window::set_title(std::string s) { title_ = std::move(s); }
std::string_view ImGuiX::Window::get_title() const { return title_; }

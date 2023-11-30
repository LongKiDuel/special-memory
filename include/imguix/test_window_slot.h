#pragma once
#include "imguix/window.h"
#include <functional>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <string>
namespace ImGuiX {

class Test_window_slot : public Window {
public:
  Test_window_slot(std::string title,
                   std::function<std::string(std::string)> call)
      : ImGuiX::Window(title), functiona_call_(call) {}
  void draw_content() {
    ImGui::InputText("Input", &input_buffer_);
    if (ImGui::Button("Try")) {
      output_buffer_ = function_call_(input_buffer_);
    }
    ImGui::Text("output: %s", output_buffer_.c_str());
  }

private:
  std::string input_buffer_;
  std::string output_buffer_;
  std::function<std::string(std::string)> function_call_;
};

} // namespace ImGuiX
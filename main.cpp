#include "imguix/font_build_queue.h"
#include "imguix/font_builder.h"
#include "imguix/font_manager.h"
#include <algorithm>
#include <filesystem>
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

std::vector<std::string> get_font_files() {
  std::vector<std::filesystem::path> files{
      std::filesystem::directory_iterator{"."}, {}};
  std::vector<std::string> result;
  for (auto f : files) {
    if (f.extension() == ".ttf") {
      result.push_back(f.string());
    }
  }
  return result;
}
void paint() {
  static ImGuiX::Font_manager manage;
  std::string content = R"**(
ImGui::Text(u8"こんにちは！テスト %d", 123);
if (ImGui::Button(u8"ロード"))
{
    // do stuff
}
ImGui::InputText("string", buf, IM_ARRAYSIZE(buf));
ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
人人有權享有生命
人人生而自由
發布掰個世界人權宣言
국가간에 우호관계의
인간은
인간이 폭정과 억압에
  )**";
  ImGuiX::Font_builder builder;
  builder.add(content);

  static std::vector<std::string> fonts = get_font_files();
  static std::string *selected = fonts.data();
  if (fonts.empty()) {
    ImGui::Text("No fonts");
    return;
  }
  if (ImGui::BeginCombo("fonts", selected ? selected->data() : "NULL")) {
    for (auto &f : fonts) {
      if (ImGui::Selectable(f.c_str(), selected == &f)) {
        selected = &f;
      }
    }
    ImGui::EndCombo();
  }
  if (int wheel = -ImGui::GetIO().MouseWheel; ImGui::IsItemHovered() && wheel) {
    selected += wheel;
    selected =
        std::clamp(selected, fonts.data(), fonts.data() + fonts.size() - 1);
  }
  ImGuiX::Font_info info{};
  info.file_path_ = *selected;
  info.ranges_ = builder.get_range();
  info.size_ = 27;
  info.config_ = ImGuiX::default_font_config();
  auto raii_handle = manage.raii_push(info);
  if (!manage.lookup(info)) {
    ImGuiX::details::build_queue.queue_.push_back(
        [info] { manage.build(info); });
  }
  ImGui::Text("%s", content.c_str());
}
int main() {
  auto app = ImGuiX::create_vulkan_app();

  // app->add_window(std::make_shared<Window_slot>("Table", table));
  app->add_window(std::make_shared<Window_slot>("Draw", paint));

  app->run();
}

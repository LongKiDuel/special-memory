#include "imguix/font_build_queue.h"
#include "imguix/font_builder.h"
#include "imguix/font_manager.h"
#include <algorithm>
#include <filesystem>
#include <optional>
#include <string_view>
#define IMGUI_DEFINE_MATH_OPERATORS
///
#include "imguix/window.h"
#include "include/imguix/app.h"
#include <cmath>
#include <functional>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <memory>
#include <string>
#include <vector>

void colorful_text(std::string_view str) {
  auto rd = str.find("RED");
  ImVec4 color = {1, 0, 0, 1};
  if (rd == -1) {
    ImGui::Text("%s", std::string{str}.c_str());
  } else {
    ImGui::Text("%s", std::string{str.substr(0, rd)}.c_str());
    str = str.substr(rd + 3);
    std::string_view red_part;
    auto wd = str.find("WHITE");
    if (wd == -1) {
      red_part = str;
    } else {
      red_part = str.substr(0, wd);
    }

    ImGui::SameLine(0, 0);
    ImGui::TextColored(color, "%s", std::string{red_part}.c_str());

    if (wd != -1) {
      str = str.substr(wd + 5);
      ImGui::SameLine(0, 0);
      colorful_text(str);
    }
  }
}
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
Whereas disregard and contempt for human rights have resulted
  )**";
  std::string color_text = R"**(
The code in imgui.cpp embeds a copy of 'ProggyClean.ttf' (by Tristan Grimmer), a RED13WHITE pixels high, pixel-perfect font used by default. We embed it in the source code so you can use Dear ImGui without any file system access. ProggyClean does not scale smoothly, therefore it is recommended that you load your own file when using Dear ImGui in an application aiming to look nice and wanting to support multiple resolutions.
  )**";
  ImGuiX::Font_builder builder;
  builder.add(content);
  builder.add(color_text);
  builder.add('?');

  static std::vector<std::string> fonts = get_font_files();
  static std::string *selected = fonts.data();
  static std::string input;
  static int font_size = 32;

  builder.add(input);

  builder.add("MHP:/");
  builder.add(ImGui::GetIO().Fonts->GetGlyphRangesDefault());
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
  info.size_ = font_size;
  info.config_ = ImGuiX::default_font_config();
  // info.config_.RasterizerDensity = 2;
  // info.config_.MergeMode = !manage.empty(); merge mode.

  ImGui::InputInt("Font size", &font_size);
  if (ImGui::Button("Clean atlas")) {
    ImGuiX::details::build_queue.queue_.push_back([info] {
      auto &io = ImGui::GetIO();
      io.Fonts->Clear();
      // static ImFontAtlas *old{};
      // if (old) {
      //   std::swap(old, io.Fonts);
      // } else {
      //   old = io.Fonts;
      //   io.Fonts = new ImFontAtlas;
      // }
      manage = {};
      manage.build(info);
    });
  }

  if (ImGui::Button("Set to default")) {
    auto font = manage.lookup(info);
    ImGui::GetIO().FontDefault = font;
  }
  auto raii_handle = manage.raii_push(info);
  if (!manage.lookup(info)) {
    ImGuiX::details::build_queue.queue_.push_back(
        [info] { manage.build(info); });
  }
  ImGui::Text("%s", content.c_str());
  colorful_text(color_text);

  ImGui::Text("HP: ");
  ImGui::PushStyleColor(ImGuiCol_Text, {0, 1, 0, 1});
  ImGui::SameLine();
  ImGui::Text("1523");
  ImGui::PopStyleColor();

  ImGui::Text("MP: ");
  ImGui::PushStyleColor(ImGuiCol_Text, {0.2, 0.5, 0.9, 1});
  ImGui::SameLine();
  ImGui::Text("89/100");
  ImGui::PopStyleColor();

  ImGui::InputText("Input", &input);
  ImGui::TextWrapped("%s", input.c_str());
}
int main() {
  auto app = ImGuiX::create_vulkan_app();

  // app->add_window(std::make_shared<Window_slot>("Table", table));
  app->add_window(std::make_shared<Window_slot>("Draw", paint));

  app->run();
}

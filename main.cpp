#include "imguix/window.h"
#include "include/imguix/app.h"
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
int main() {
  auto app = ImGuiX::create_vulkan_app();

  auto call = []() {
    static std::vector<std::vector<int>> content = [] {
      std::vector<std::vector<int>> content;
      for (int i = 0; i < 5; i++) {
        content.push_back({});
        for (int j = 0; j < 5; j++) {
          content.back().push_back(i * i + j);
        }
      }
      return content;
    }();
    ImGuiTableFlags flag = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable |
                           ImGuiTableFlags_Reorderable;
    ImGui::BeginTable("const char *str_id", content[0].size() + 1, flag);
    int id{};
    for (int i = 0; i < content[0].size() + 1; i++) {
      if (i == 0) {
        ImGui::TableSetupColumn("");
      } else {
        ImGui::TableSetupColumn(std::string(1, 'a' + i - 1).c_str());
      }
      ImGui::NextColumn();
    }
    ImGui::TableHeadersRow();
    int rowID{};
    for (auto &row : content) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("%s", std::to_string(rowID + 1).c_str());
      for (auto &c : row) {
        ImGui::PushID(++id);
        ImGui::TableNextColumn();

        ImGui::DragInt("", &c);
        ImGui::PopID();
      }
      rowID++;
    }
    ImGui::EndTable();
  };
  app->add_window(std::make_shared<Window_slot>("Table", call));

  app->run();
}

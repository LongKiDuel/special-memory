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
      const int si = 25;
      std::vector<std::vector<int>> content;
      for (int i = 0; i < si; i++) {
        content.push_back({});
        for (int j = 0; j < si; j++) {
          content.back().push_back(i * i + j);
        }
      }
      return content;
    }();
        ImGui::TableNextColumn();
    ImGuiTableFlags flag = ImGuiTableFlags_Borders |
                           ImGuiTableFlags_Reorderable |
                           ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
                           ImGuiTableFlags_SizingStretchProp;
    const int table_width = 1000;
    ImGui::BeginTable("const char *str_id", content[0].size() + 1, flag,
                      ImVec2(table_width, 500));
    ImGui::TableSetupScrollFreeze(1, 1);
    int id{};
    for (int i = 0; i < content[0].size() + 1; i++) {
      if (i == 0) {
        ImGui::TableSetupColumn(
            "", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthFixed,
            25);
      } else {
        ImGui::TableSetupColumn(std::string(1, 'a' + i - 1).c_str(),
                                ImGuiTableColumnFlags_WidthFixed, 50);
      }
    }
    auto total_col = content[0].size() + 1;
    ImGui::TableHeadersRow();
    int rowID{};
    for (auto &row : content) {
      ImGui::TableNextRow();
      if (rowID == 0) {
        for (int i = 0; i < total_col; i++) {
          ImGui::TableSetColumnIndex(i);
          ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
        }
      }
      ImGui::TableNextColumn();
      ImGui::Text("%s", std::to_string(rowID + 1).c_str());
      for (auto &c : row) {
        if (ImGui::TableNextColumn()) {
          ImGui::PushID(++id);
          ImGui::DragInt("", &c);
          ImGui::PopID();
        }
      }
      rowID++;
    }
    ImGui::EndTable();
  };
  app->add_window(std::make_shared<Window_slot>("Table", call));

  app->run();
}

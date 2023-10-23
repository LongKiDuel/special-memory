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
void table() {
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
  ImGuiTableFlags flag = ImGuiTableFlags_Borders | ImGuiTableFlags_Reorderable |
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
}
void paint() {
  struct Canvas_context {
    ImVec2 canvas_offset_{};
    bool draw_grid_{true};
    float grid_size_{32.f};
  };
  static Canvas_context context;
  ImGui::Text("offset: %f %f", context.canvas_offset_.x,
              context.canvas_offset_.y);

  const ImVec2 offset = ImGui::GetCursorScreenPos();
  const ImVec2 size = ImGui::GetContentRegionAvail();

  const ImVec2 min_position = offset;
  const ImVec2 max_postion = {offset.x + size.x, offset.y + size.y};

  auto draw_list = ImGui::GetWindowDrawList();
  draw_list->AddRectFilled(min_position, max_postion,
                           IM_COL32(50, 50, 50, 255));
  draw_list->AddRect(min_position, max_postion, IM_COL32(255, 255, 255, 255));

  ImGui::InvisibleButton("canvas", size,
                         ImGuiButtonFlags_MouseButtonLeft |
                             ImGuiButtonFlags_MouseButtonRight |
                             ImGuiButtonFlags_MouseButtonMiddle);
  const bool is_hover = ImGui::IsItemHovered();
  const bool is_active = ImGui::IsItemActive();

  const ImVec2 origin{min_position.x + context.canvas_offset_.x,
                      min_position.x + context.canvas_offset_.y};

  const auto &io = ImGui::GetIO();
  const ImVec2 mouse_position_in_canvas{io.MousePos.x - origin.x,
                                        io.MousePos.y - origin.y};

  if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
    context.canvas_offset_.x += io.MouseDelta.x;
    context.canvas_offset_.y += io.MouseDelta.y;
  }

  draw_list->PushClipRect(min_position, max_postion, true);

  if (context.draw_grid_) {
    const auto grid_size = context.grid_size_;
    const auto grid_color = IM_COL32(150, 150, 150, 100);

    auto draw_line = [&](float x1, float x2, float y1, float y2) {
      draw_list->AddLine(ImVec2{x1, y1}, ImVec2(x2, y2), grid_color);
      if (y1 == y2) {
        auto canvas_y = y1 - min_position.y;
        auto str = "Screen pos: "+ std::to_string(y1)+ " canvas position: " + std::to_string(canvas_y) + " abs: " +
                   std::to_string(canvas_y - context.canvas_offset_.y);
        draw_list->AddText(ImVec2(x1, y1), -1, str.c_str());
      }
    };

    for (auto x = std::fmod(context.canvas_offset_.x, grid_size); x < size.x;
         x += grid_size) {
      draw_line(min_position.x + x, min_position.x + x, min_position.y,
                max_postion.y);
    }
    for (auto y = std::fmod(context.canvas_offset_.y, grid_size); y < size.y;
         y += grid_size) {
      draw_line(min_position.x, max_postion.x, min_position.y + y,
                min_position.y + y);
    }
  }

  draw_list->PopClipRect();
}
int main() {
  auto app = ImGuiX::create_vulkan_app();

  // app->add_window(std::make_shared<Window_slot>("Table", table));
  app->add_window(std::make_shared<Window_slot>("Draw", paint));

  app->run();
}

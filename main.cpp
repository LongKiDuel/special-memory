#define IMGUI_DEFINE_MATH_OPERATORS
///
#include "imgui_sink.h"
#include "imguix/window.h"
#include "include/imguix/app.h"
#include <cmath>
#include <functional>
#include <imgui.h>
#include <memory>
#include <optional>
#include <spdlog/spdlog.h>
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

  // world coord
  // paint coord
  // screen coord
  struct Canvas_context {
    ImVec2 canvas_offset_{};
    bool draw_grid_{true};
    float grid_size_{32.f};
    float scale_factor_{1.f};

    ImVec2 canvas_size_in_screen_;
    ImVec2 canvas_offset_in_screen_;

    ImVec2 get_position() const { return canvas_offset_in_screen_; }

    ImVec2 get_canvas_size() const { return canvas_size_in_screen_; }
    ImVec2 get_paint_size() const { return get_canvas_size() / scale_factor_; }
    ImVec2 get_scanvas_min_position() const { return canvas_offset_in_screen_; }
    ImVec2 get_canvas_max_position() const {
      return get_canvas_size() + get_scanvas_min_position();
    }
    ImVec2 get_paint_min_position() const {
      return get_paint_center() - get_paint_size() / 2;
    }
    ImVec2 get_paint_max_position() const {
      return get_paint_center() + get_paint_size() / 2;
    }
    // normalized range is [-1,1]
    ImVec2 paint_position_normalize(ImVec2 paint_position) {
      return ((paint_position - get_paint_center()) / get_paint_size()) * 2;
    }
    ImVec2 paint_position_de_normalize(ImVec2 normalized_pos) {
      return normalized_pos / 2 * get_paint_size() + get_paint_center();
    }
    ImVec2 paint_position_to_screen(ImVec2 paint_position) {
      return normalized_position_to_screen(
          paint_position_normalize(paint_position));
    }
    ImVec2 normalized_position_to_screen(ImVec2 position) {
      return canvas_offset_in_screen_ +
             (position / 2 + ImVec2(0.5f, 0.5f)) * get_canvas_size();
    }

    ImVec2 screen_position_normalize(ImVec2 screen_position) {
      screen_position -= canvas_offset_in_screen_;
      screen_position /= get_canvas_size();
      screen_position *= 2;
      screen_position -= ImVec2{1, 1};
      return screen_position;
    }
    ImVec2 screen_position_de_normalize(ImVec2 screen_position) {
      screen_position += ImVec2{1, 1};
      screen_position /= 2;
      screen_position *= get_canvas_size();
      screen_position += canvas_offset_in_screen_;
      return screen_position;
    }
    ImVec2 world_position_normalize(ImVec2 world_position) {
      auto paint_position = world_position - canvas_offset_;
      return paint_position_normalize(paint_position);
    }
    ImVec2 world_position_de_normalize(ImVec2 ndc) {
      return paint_position_de_normalize(ndc) + canvas_offset_;
    }

    ImVec2 get_paint_center() const { return canvas_offset_; }

    ImVec2 get_mouse_pos_in_normalized() {
      auto &io = ImGui::GetIO();
      return screen_position_normalize(io.MousePos);
    }

    void begin_frame() {
      canvas_size_in_screen_ = ImGui::GetContentRegionAvail();
      canvas_offset_in_screen_ = ImGui::GetCursorScreenPos();
    }
    void end_frame() {}
  };
  static Canvas_context context;

  const ImVec2 prev_size = context.get_paint_size();
  ImGui::Text("offset: %f %f", context.canvas_offset_.x,
              context.canvas_offset_.y);
  const ImVec2 center = context.get_paint_center();
  ImGui::Text("center: %f %f", center.x, center.y);
  ImGui::DragFloat("scale", &context.scale_factor_, 0.01, 0.01);
  if (context.scale_factor_ <= 0) {
    context.scale_factor_ = 0.01;
  }
#define XX(fun) ImGui::Text(#fun ": %f %f", context.fun.x, context.fun.y)
  XX(paint_position_normalize(center));
  XX(get_paint_min_position());
  XX(get_paint_max_position());
  XX(paint_position_normalize(context.get_paint_min_position()));
  XX(paint_position_normalize(context.get_paint_max_position()));
  context.begin_frame();

  const ImVec2 size = context.get_canvas_size();
  const ImVec2 offset = context.get_position();

  const ImVec2 min_position = context.get_scanvas_min_position();
  const ImVec2 max_postion = context.get_canvas_max_position();

  class Click_rect {
    ImVec2 begin_{};
    ImVec2 end_{};
    enum State { ready, working, done };
    State state_{ready};

  public:
    void update(bool actived, std::function<ImVec2()> pos_provider) {
      const auto &io = ImGui::GetIO();
      if (state_ == done) {
        return;
      }

      if (state_ == working) {
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
          end_ = pos_provider();
          state_ = done;
        } else {
          end_ = pos_provider();
        }
        return;
      }
      if (!actived) {
        return;
      }
      if (state_ == ready) {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
          begin_ = pos_provider();
          end_ = pos_provider();
          state_ = working;
        }
        return;
      }
    }
    struct Result {
      ImVec2 begin_pos_;
      ImVec2 end_pos_;
      ImVec2 min() {
        return {std::min(begin_pos_.x, end_pos_.x),
                std::min(begin_pos_.y, end_pos_.y)};
      }
      ImVec2 max() {
        return {std::max(begin_pos_.x, end_pos_.x),
                std::max(begin_pos_.y, end_pos_.y)};
      }
    };
    std::optional<Result> get_result() {
      if (state_ == ready) {
        return {};
      }
      return Result{begin_, end_};
    }
    void reset() { state_ = ready; }
    bool has_done() const { return state_ == done; }
  };
  static Click_rect rect{};

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
                      min_position.y + context.canvas_offset_.y};

  const auto &io = ImGui::GetIO();
  ImVec2 mouse_position_in_canvas{io.MousePos.x - origin.x,
                                  io.MousePos.y - origin.y};

  if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
    context.canvas_offset_.x += io.MouseDelta.x;
    context.canvas_offset_.y += io.MouseDelta.y;
  }
  if (is_hover && io.MouseWheel) {
    if (io.MouseWheel < 0) {
      context.scale_factor_ *= 0.9;
    } else {
      context.scale_factor_ *= 1.1;
    }
  }

  draw_list->PushClipRect(min_position, max_postion, true);

  if (context.draw_grid_) {
    const int pixel_width = 3840;
    const int pixel_height = 2096;
    const auto grid_size = context.grid_size_;
    const auto grid_color = IM_COL32(150, 150, 150, 100);

    auto draw_line = [&](float x1, float x2, float y1, float y2) {
      draw_list->AddLine(ImVec2{x1, y1}, ImVec2(x2, y2), grid_color);
    };
    auto origin = context.normalized_position_to_screen(
        context.world_position_normalize(ImVec2{}));

    ImVec2 mouse_in_world = context.world_position_de_normalize(
        context.get_mouse_pos_in_normalized());
    ImVec2 mouse_grid_id = ImVec2{std::floor(mouse_in_world.x / grid_size),
                                  std::floor(mouse_in_world.y / grid_size)};
    auto grid_id_to_canvas = [&](ImVec2 grid_id) {
      return ImVec2{grid_id.x * grid_size, grid_id.y * grid_size};
    };
    auto canvas_pos_to_screen = [&](ImVec2 canvas_position) {
      return context.normalized_position_to_screen(
          context.world_position_normalize(canvas_position));
    };
    auto grid_to_screen = [&](ImVec2 grid_id) {
      return canvas_pos_to_screen(grid_id_to_canvas(grid_id));
    };
    static std::vector<ImVec2> grid_to_fill;
    if (is_active && ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
      [&] {
        for (const auto &g : grid_to_fill) {
          if (g.x == mouse_grid_id.x && g.y == mouse_grid_id.y) {
            return;
          }
        }
        grid_to_fill.push_back(mouse_grid_id);
      }();
    }

    for (auto gid : grid_to_fill) {
      draw_list->AddRectFilled(grid_to_screen(gid),
                               grid_to_screen(gid + ImVec2{1, 1}), -1);
    }
    draw_list->AddCircleFilled(canvas_pos_to_screen(mouse_position_in_canvas),
                               10, IM_COL32(0, 0, 0, 255));

    for (int i = 0; i * grid_size <= pixel_width; i++) {
      auto px = i * grid_size;
      auto v = context.normalized_position_to_screen(
          context.world_position_normalize(ImVec2{float(px), pixel_height}));
      draw_line(v.x, v.x, origin.y, v.y);
    }
    for (int i = 0; i * grid_size <= pixel_height; i++) {
      auto py = i * grid_size;
      auto v = context.normalized_position_to_screen(
          context.world_position_normalize(ImVec2{pixel_width, py}));
      draw_line(origin.x, v.x, v.y, v.y);
    }
    rect.update(is_active, [&]() { return mouse_grid_id; });
    static std::optional<Click_rect::Result> prev_result;
    {
      auto result = rect.get_result();
      if (result) {
        prev_result = result;
      }
      if (rect.has_done()) {
        rect.reset();
        auto min = prev_result->min();
        auto max = prev_result->max();

        for (int y = min.y; y < max.y; y++) {
          for (int x = min.x; x < max.x; x++) {
            grid_to_fill.push_back(ImVec2(x, y));
          }
        }
      }
      if (prev_result) {
        draw_list->AddRect(grid_to_screen(prev_result->begin_pos_),
                           grid_to_screen(prev_result->end_pos_),
                           IM_COL32(255, 0, 0, 255), 0, 0, 5);
      }
    }
    draw_list->AddCircleFilled(
        context.paint_position_to_screen(context.get_paint_center()), 10,
        IM_COL32(255, 0, 0, 255));
    draw_list->AddCircleFilled(
        context.normalized_position_to_screen(
            context.screen_position_normalize(io.MousePos)),
        10, IM_COL32(0, 255, 0, 255));

    auto world_origin = context.normalized_position_to_screen(
        context.world_position_normalize(ImVec2{0, 0}));
    draw_list->AddCircleFilled(world_origin, 10, IM_COL32(255, 255, 255, 255));

    draw_list->AddLine(world_origin - ImVec2{5000, 0},
                       world_origin + ImVec2{5000, 0}, IM_COL32(155, 0, 0, 255),
                       3);
    draw_list->AddLine(world_origin - ImVec2{0, 5000},
                       world_origin + ImVec2{0, 5000}, IM_COL32(0, 155, 0, 255),
                       3);
  }

  draw_list->PopClipRect();
}

void wrapping_test() {
  static float box_size = 20;
  static int box_count = 40;
  ImGui::DragFloat("Box size", &box_size);
  ImGui::DragInt("Box count", &box_count);

  for (auto i = 0; i < box_count; i++) {
    if (ImGui::GetContentRegionAvail().x < box_size && (i != 0)) {
      ImGui::NewLine();
    }

    ImGui::Button(std::to_string(i).c_str(), {box_size, box_size});
    ImGui::GetItemRectSize();
    ImGui::SameLine();
  }
}
int main() {
  auto app = ImGuiX::create_vulkan_app();

  // app->add_window(std::make_shared<Window_slot>("Table", table));
  app->add_window(std::make_shared<Window_slot>("Draw", paint));
  app->add_window(std::make_shared<Window_slot>("Wrapping", wrapping_test));

  app->add_window(std::make_shared<Window_slot>("Tasking", [] {
    if (ImGui::Button("Send")) {
      SPDLOG_INFO("send log success!");
    }
  }));
  Imgui_log_window log;
  app->add_window(
      std::make_shared<Window_slot>("Logging", [&log] { log.draw(); }));

  app->run();
}

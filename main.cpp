#include <deque>
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

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
class Matrix_unit {
public:
  ~Matrix_unit() = default;
  virtual glm::mat4 get_mat() const = 0;
  virtual void draw() = 0;
  virtual const std::string &name() const = 0;
};

class Plain_matrix : public Matrix_unit {
public:
  Plain_matrix() {}
  Plain_matrix(std::string name, glm::mat4 mat) {
    name_ = name;
    mat_ = mat;
  }

  glm::mat4 get_mat() const { return mat_; }
  void draw() {}
  const std::string &name() const { return name_; }

  std::string name_;
  glm::mat4 mat_;
};
void draw_mat(std::string name, const glm::mat4 &mat) {
  const float *cursor = &mat[0][0];
  ImGui::Text("%s:", name.c_str());
  if (ImGui::BeginTable(name.c_str(), 4)) {
    for (int i = 0; i < 4 * 4; i++) {
      if (i % 4 == 0) {
        ImGui::TableNextRow();
      }
      ImGui::TableNextColumn();
      ImGui::Text("%.2f", cursor[i]);
    }
    ImGui::EndTable();
  }
}

class Matrix_system {
public:
  void append(std::shared_ptr<Matrix_unit> unit) { matrices_.push_front(unit); }
  void prepend(std::shared_ptr<Matrix_unit> unit) { matrices_.push_back(unit); }

  void draw() {
    ImGui::Text("Matrix system:");
    if (ImGui::BeginTabBar("Matrices")) {
      int index{};
      for (auto &mat : matrices_) {

        if (ImGui::BeginTabItem(mat->name().c_str())) {
          const auto mat4 = mat->get_mat();
          draw_mat("mat", mat4);
          current_index_ = index;
          auto result = get_current_mat();
          draw_mat("result", result);

          mat->draw();
          ImGui::EndTabItem();
        }
        index++;
      }
      ImGui::EndTabBar();
    }
  }
  glm::mat4 get_current_mat() {
    glm::mat4 result{1};
    for (int i = matrices_.size() - 1; i >= current_index_; i--) {
      result = matrices_[i]->get_mat() * result;
    }
    return result;
  }

private:
  std::deque<std::shared_ptr<Matrix_unit>> matrices_;
  int current_index_{};
};
#include <ImGuizmo.h>
void draw_cube(glm::mat4 mvp) {
  auto draw = ImGui::GetForegroundDrawList();
  std::vector<glm::vec4> points = {
      {1, 0, -1, 1},
      {2, 0, 0, 1},
      {1, 0, 1, 1},
  };
  std::vector<ImVec2> result_points;
  auto frame_size = ImGui::GetMainViewport()->Size;

  for (auto p : points) {
    auto p2 = mvp * p;
    p2 /= p2.w;
    // ImGui::Text("[%f %f %f %f]", p2.x, p2.y, p2.z, p2.w);
    p2 += 1.;
    p2 /= 2;
    p2.x *= frame_size.x;
    p2.y *= frame_size.y;
    // p2 /= p2.z;
    result_points.push_back({p2.x, p2.y});
    ImGui::Text("[%f %f %f %f]", p2.x, p2.y, p2.z, p2.w);
  }
  for (int i = 0; i + 3 <= result_points.size(); i += 3) {
    draw->AddTriangleFilled(result_points[i], result_points[i + 1],
                            result_points[i + 2], -1);
  }
}
void paint() {
  static Matrix_system system;
  static bool b = []() {
    auto mat = std::make_shared<Plain_matrix>();
    mat->name_ = "basic";
    mat->mat_ = glm::mat4{1};
    system.append(mat);
    {
      auto move = std::make_shared<Plain_matrix>(
          "move", glm::translate(glm::mat4{1}, glm::vec3{10, 20, 30}));

      // system.append(move);
    }
    return true;
  }();
  ImGuizmo::BeginFrame();
  static auto view = glm::lookAtRH(glm::vec3{10, 10, 10}, {}, {0, 1, 0});
  static auto projection = glm::perspectiveFovRH_ZO(
      1.f, ImGui::GetMainViewport()->Size.x, ImGui::GetMainViewport()->Size.y,
      0.001f, 1000.f);
  static glm::mat4 mat;
  mat = system.get_current_mat();
  ImGuizmo::ViewManipulate(&view[0][0], 10, {10, 10}, {300, 300}, 0);
  draw_cube(projection * view * mat);

  system.draw();
}
int main() {
  auto app = ImGuiX::create_vulkan_app();

  // app->add_window(std::make_shared<Window_slot>("Table", table));
  app->add_window(std::make_shared<Window_slot>("Draw", paint));

  app->run();
}

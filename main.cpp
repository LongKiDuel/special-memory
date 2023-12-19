#include <array>
#include <deque>
#include <glm/ext/matrix_clip_space.hpp>
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
#include <glm/gtx/quaternion.hpp>
class Quat_rotate_matrix : public Matrix_unit {
public:
  glm::mat4 get_mat() const {
    std::array<glm::vec4, 4> vec{};
    for (int i = 0; i < 3; i++) {
      glm::vec3 origin = {};
      origin[i] = 1;
      auto new_dir = glm::normalize(glm::rotate(get_quat(), origin));
      vec[i] = {new_dir, 0};
    }
    vec[3][3] = 1;
    glm::mat4 m{vec[0], vec[1], vec[2], vec[3]};
    return m;
  }
  glm::quat get_quat() const {
    // return orbit1_ * t + (1 - t) * orbit2_;
    float co1 = std::sin(1 - t) / std::sin(t);
    float co2 = std::sin(t) / std::sin(t);
    return co1 * orbit1_ + co2 * orbit2_;
  }
  void draw() {
    t += 0.01;
    if (t > 1) {
      t = 0;
    }
    ImGui::Text("Time: %f", t);
    ImGui::DragFloat4("quater 1", &orbit1_.x);
    ImGui::DragFloat4("quater 2", &orbit2_.x);
  }

  const std::string &name() const {
    static std::string str = "Quat rotate";
    return str;
  }

  glm::quat orbit1_{1, 1, 1, 1};
  glm::quat orbit2_{1, 0, 1, 1};
  float t{};
};
class Camera_matrix : public Matrix_unit {
public:
  glm::mat4 get_mat() const {
    return glm::lookAt(get_position(), center_, up_);
  }
  glm::vec3 get_position() const {
    auto dir = glm::rotate(orbit_, {0, 1, 0});
    dir = glm::normalize(dir);
    return center_ + dir * distance_;
  }
  void draw() {
    ImGui::DragFloat4("quater", &orbit_.x);
    ImGui::DragFloat("distance", &distance_);
    auto pos = get_position();
    ImGui::Text("Position: (%f %f %f)", pos.x, pos.y, pos.z);
  }

  const std::string &name() const {
    static std::string str = "Camera";
    return str;
  }

  glm::quat orbit_{1, 1, 1, 1};
  glm::vec3 center_{};
  float distance_{10};
  glm::vec3 up_{0, 1, 0};
};
class Project_matrix : public Matrix_unit {
public:
  Project_matrix() { update(); }
  glm::mat4 get_mat() const {
    return glm::perspectiveFovRH_ZO<float>(fov_, width_, height_, 0.01, 1000);
  }

  const std::string &name() const {
    static std::string str = "Project";
    return str;
  }
  void draw() { update(); }
  void update() {
    auto frame_size = ImGui::GetMainViewport()->Size;
    width_ = frame_size.x;
    height_ = frame_size.y;
  }
  float fov_{glm::radians(45.f)};
  float width_{};
  float height_{};
};

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
  glm::mat4 get_final_mat() {
    glm::mat4 result{1};
    for (int i = matrices_.size() - 1; i >= 0; i--) {
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
      system.append(std::make_shared<Quat_rotate_matrix>());
    }
    {
      system.append(std::make_shared<Camera_matrix>());
      system.append(std::make_shared<Project_matrix>());
    }
    return true;
  }();
  ImGuizmo::BeginFrame();

  static glm::mat4 mat;
  mat = system.get_final_mat();
  // ImGuizmo::ViewManipulate(&view[0][0], 10, {10, 10}, {300, 300}, 0);
  draw_cube(mat);

  system.draw();
}
int main() {
  auto app = ImGuiX::create_vulkan_app();

  // app->add_window(std::make_shared<Window_slot>("Table", table));
  app->add_window(std::make_shared<Window_slot>("Draw", paint));

  app->run();
}

#include <array>
#include <cstdint>
#include <deque>
#include <fstream>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/matrix_float3x4.hpp>
#include <glm/ext/matrix_float4x3.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/geometric.hpp>
#include <glm/matrix.hpp>
#include <iostream>
#include <optional>
#include <span>
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
    if (ImGui::Button("Nomalize")) {
      orbit1_ = glm::normalize(orbit1_);
      orbit2_ = glm::normalize(orbit2_);
    }
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
namespace graphic_compute {
class Plane {
public:
  Plane(glm::vec3 normal, float distance) : plane_vector_(normal, distance) {}

  glm::vec3 normal() const { return glm::vec3{plane_vector_}; }
  float distance() const { return plane_vector_.w; }

  glm::vec4 vec4_form() const { return plane_vector_; }

private:
  glm::vec4 plane_vector_{};
};
class Line {
public:
  Line(glm::vec3 position, glm::vec3 direction)
      : line_point_(position), direction_(glm::normalize(direction)) {}
  glm::vec3 point() const { return line_point_; }
  glm::vec3 direction() const { return direction_; }

private:
  glm::vec3 line_point_;
  glm::vec3 direction_;
};

class Triangle {
public:
  Triangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) : points_{p1, p2, p3} {}
  glm::vec3 center() const {
    glm::vec3 p{};
    for (auto &tp : points_) {
      p += tp;
    }
    p /= 3;
    return p;
  }
  glm::vec3 normal() const {
    auto result = glm::normalize(
        glm::cross(points_[1] - points_[0], points_[2] - points_[0]));
    return result;
  }

  glm::vec3 operator[](int index) const { return points_[index]; }

  glm::vec3 &operator[](int index) { return points_[index]; }

private:
  std::array<glm::vec3, 3> points_{};
};

Plane triangle_plane(const Triangle &triangle) {
  auto normal = triangle.normal();
  auto p0 = triangle[0];
  auto distance = glm::dot(-normal, p0);
  return Plane{normal, distance};
}

std::optional<float> intersect(const Line &line, const Plane &plane) {
  auto line_dir = glm::vec4{line.direction(), 0};
  auto line_pos = glm::vec4{line.point(), 1};

  auto plane_vector = plane.vec4_form();

  auto direction_dot = glm::dot(plane_vector, line_dir);
  if (direction_dot == 0) {
    return {};
  }
  auto distance = glm::dot(plane_vector, line_pos);
  return -distance / direction_dot;
}
bool check_point_in_triangle(const glm::vec3 &point, const Triangle &triangle) {
  // transform triangle[0] into origin.
  auto q1 = triangle[1] - triangle[0];
  auto q2 = triangle[2] - triangle[0];
  auto r = point - triangle[0];
  // barycentric coordinate.
  std::array<float, 3> w{};
  auto &w1 = w[1];
  auto &w2 = w[2];
  auto w0 = [&w1, &w2]() { return 1 - w1 - w2; };
  // compute
  auto dot_of_q1_q2 = glm::dot(q1, q2);
  auto q1_square = glm::dot(q1, q1);
  auto q2_square = glm::dot(q2, q2);

  glm::vec2 coord = {glm::dot(r, q1), glm::dot(r, q2)};
  glm::mat2 mat = {q2_square, -dot_of_q1_q2, -dot_of_q1_q2, q1_square};
  float factor = 1 / (q1_square * q2_square - std::pow(dot_of_q1_q2, 2));
  auto w1_w2_vec = factor * (mat * coord);
  w1 = w1_w2_vec.x;
  w2 = w1_w2_vec.y;
  w[0] = w0();

  for (auto v : w) {
    if (v < 0 || v > 1) {
      return false;
    }
  }
  return true;
}
template <typename T> struct Average_reducer {
  void operator()(T value) {
    sum += value;
    count++;
  }
  T get_average() { return sum / count; }
  T sum{};
  int64_t count{};
};
glm::vec3 average(std::span<glm::vec3> points) {
  if (points.empty()) {
    throw std::runtime_error("The set of points is empty.");
  }

  glm::vec3 sum(0.0f, 0.0f, 0.0f);

  for (const auto &point : points) {
    sum += point;
  }

  return sum / static_cast<float>(points.size());
}
// for compute natural alignment, need other step to compute from this result.
glm::mat3 convariance_matrix(std::span<glm::vec3> points) {
  auto m = average(points);
  float c11{}, c22{}, c33{}, c12{}, c13{}, c23{};
  auto square = [](auto n) { return std::pow(n, 2); };
  for (auto p : points) {

    auto dx = p.x - m.x;
    auto dy = p.y - m.y;
    auto dz = p.z - m.z;
    c11 += square(dx);
    c22 += square(dy);
    c33 += square(dz);
    c12 += dx * dy;
    c13 += dx * dz;
    c23 += dy * dz;
  }
  auto size = points.size();
  c11 /= size;
  c12 /= size;
  c13 /= size;
  c22 /= size;
  c23 /= size;
  c33 /= size;
  return glm::mat3{c11, c12, c13, c12, c22, c23, c13, c23, c33};
}
class Sphere {
public:
  Sphere(glm::vec3 center, float radius) : center_(center), radius_(radius) {}
  Sphere() {}
  glm::vec3 center() const { return center_; }
  float radius() { return radius_; }

private:
  glm::vec3 center_{};
  float radius_{};
};

Sphere computer_bouding_sphere(std::span<const glm::vec3> points) {
  if (points.empty()) {
    return {};
  }

  for (const auto &point : points) {
  }

  return {};
}
class Polynominal {};
class Cubic_curve {
public:
  Cubic_curve(glm::vec3 c0, glm::vec3 c1, glm::vec3 c2, glm::vec3 c3)
      : cofactors_{c0, c1, c2, c3} {}
  glm::vec3 operator()(float t) const {
    auto mat = coeeficient_matrix();
    glm::vec4 vec_t{1, t, t * t, t * t * t};
    return mat * vec_t;
  }
  glm::mat3x4 coeeficient_matrix() const {
    auto &c = cofactors_;
    glm::mat4x3 mat_t{c[0], c[1], c[2], c[3]};
    auto mat = glm::transpose(mat_t);
    return mat;
  }

  glm::vec3 derivative(float t) const {
    auto mat = coeeficient_matrix();
    glm::vec4 vec_t{0, 1, 2 * t, 3 * t * t};
    return mat * vec_t;
  }

private:
  std::array<glm::vec3, 4> cofactors_{};
};
class Hermite_curve {
  glm::vec3 operator()(float t) const { return {}; }

private:
  Line p1_;
  Line p2_;
};
class Bezier_curve {
public:
  glm::vec3 operator()(float t) const { return {}; }
};

class Cubic_Bezier_curve {
public:
  Cubic_Bezier_curve(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
      : points_{p0, p1, p2, p3} {}
  glm::vec3 operator()(float t) const {
    auto dt = 1 - t;
    auto &p = points_;
    return p[0] * std::pow(dt, 3.f) + 3 * t * dt * dt * p[1] +
           3 * t * t * dt * p[2] + std::pow(t, 3.f) * p[3];
  }

private:
  std::array<glm::vec3, 4> points_{};
};
class NURBS {

  int n() { return points_.size() - 1; }
  int knot_count() { return n() + 4; }
  int i_begin() { return 1; }
  int i_end() { return n() - 2; }
  // Cox-de Boor algorithm
  float blending(int i, int k, float u) {
    auto knot_n2 = get_knot(i - 2);
    auto knot_n1 = get_knot(i - 1);

    if (k == 0) {
      if (u >= knot_n2 && u < knot_n1) {
        return 1;
      } else {
        return 0;
      }
    }
    auto linear_part1 =
        (u - knot_n2) * blending(i, k - 1, u) / (get_knot(i + k - 2) - knot_n2);
    auto linear_part2 = (get_knot(i + k - 1) - u) * blending(i + 1, k - 1, u) /
                        (get_knot(i + k - 1) - knot_n1);
    return linear_part1 + linear_part2;
  }
  float get_knot(int index) { return knots_[index + 2]; }
  std::vector<glm::vec3> points_;
  std::vector<float> knots_;
  std::vector<float> weights_;
};
} // namespace graphic_compute
void draw_mat3(std::string name, glm::mat3 mat) {
  ImGui::Text("%s: ", name.c_str());
  for (int i = 0; i < 3; i++) {
    auto &m = mat[i];
    ImGui::Text("[%f %f %f]", m[0], m[1], m[2]);
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

  std::vector<glm::vec3> poitns = {
      {-1, -2, 1},
      {1, 0, 2},
      {2, -1, 3},
      {2, -1, 2},
  };
  auto mat3 = graphic_compute::convariance_matrix(poitns);
  draw_mat3("convariance", mat3);
}
void ExportLineToObj(std::span<glm::vec3> line, const std::string &filename) {
  std::ofstream file(filename);

  if (!file.is_open()) {
    std::cerr << "Failed to open file: " << filename << std::endl;
    return;
  }

  // Write vertices
  for (const auto &vertex : line) {
    file << "v " << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;
  }

  // If the line represents a polyline, you can optionally write lines (edges)
  // for (size_t i = 0; i < line.size() - 1; ++i) {
  //     file << "l " << (i + 1) << " " << (i + 2) << std::endl;
  // }

  file.close();
}
glm::vec2 to_glm(ImVec2 vec) { return {vec.x, vec.y}; }
glm::vec4 to_glm(ImVec4 vec) { return {vec.x, vec.y, vec.z, vec.w}; }
ImVec2 to_imvec(glm::vec2 vec) { return {vec.x, vec.y}; }
ImVec4 to_imvec(glm::vec4 vec) { return {vec.x, vec.y, vec.z, vec.w}; }
void collision_test() {
  auto pos = to_glm(ImGui::GetWindowPos());
  auto size = to_glm(ImGui::GetWindowSize());

  auto draw = ImGui::GetForegroundDrawList();
  draw->AddRect(to_imvec(pos), to_imvec(pos + size), -1);
  static graphic_compute::Sphere sphere({pos, 1}, 3.f);
}

int main() {
  auto app = ImGuiX::create_vulkan_app();

  // app->add_window(std::make_shared<Window_slot>("Table", table));
  app->add_window(std::make_shared<Window_slot>("Draw", paint));
  app->add_window(std::make_shared<Window_slot>("Collision", collision_test));

  app->run();
}

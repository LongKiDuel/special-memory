#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cassert>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
class Gl_shader {
public:
  Gl_shader() : id_(0) {}
  Gl_shader(const Gl_shader &) = delete;
  Gl_shader(Gl_shader &&rhs) {
    if (id_) {
      glDeleteShader(id_);
    }
    id_ = rhs.release();
  }
  Gl_shader &operator=(Gl_shader &&rhs) {
    if (id_) {
      glDeleteShader(id_);
    }
    id_ = rhs.release();
    return *this;
  }

  Gl_shader(int type, std::string_view code) {
    auto sid = glCreateShader(type);
    id_ = sid;
    auto data = code.data();
    int len = code.size();
    glShaderSource(sid, 1, &data, &len);

    glCompileShader(id_);
  }
  ~Gl_shader() {
    if (id_) {
      glDeleteShader(id_);
    }
  }
  uint32_t get_id() const { return id_; }
  uint32_t release() {
    auto id = id_;
    id_ = 0;
    return id;
  }
  explicit operator bool() const { return id_; }

private:
  uint32_t id_{};
};

struct Gl_pipe {
  Gl_shader vertex_shader;
  std::optional<Gl_shader> geometry_shader;
  Gl_shader fragment_shader;

  bool add_vertex_shader(std::string_view code) {
    vertex_shader = Gl_shader(GL_VERTEX_SHADER, code);
    return true;
  }
  bool add_fragment_shader(std::string_view code) {

    fragment_shader = Gl_shader(GL_FRAGMENT_SHADER, code);
    return true;
  }
  bool add_geometry_shader(std::string_view code) {
    geometry_shader = Gl_shader(GL_GEOMETRY_SHADER, code);
    return true;
  }
};
class Gl_program {
public:
  Gl_program(Gl_pipe &pipe) {
    id = glCreateProgram();
    link(pipe.vertex_shader);
    if (pipe.geometry_shader) {
      link(*pipe.geometry_shader);
    }
    link(pipe.fragment_shader);

    glLinkProgram(id);
    glValidateProgram(id);
  }
  ~Gl_program() {
    if (id) {
      glDeleteProgram(id);
    }
  }
  bool link(Gl_shader &shader) {
    assert(id);
    if (!shader.get_id()) {
      return false;
    }
    glAttachShader(id, shader.get_id());
    return true;
  }
  auto release() {
    auto i = id;
    id = 0;
    return i;
  }

private:
  int id{};
};
int main() { return 0; }

#pragma once

#include <array>
#include <functional>
#include <glm/ext/vector_float3.hpp>
namespace data_stream {
struct Point_ref {
  glm::vec3 *point;
};
struct Point_stream {
  bool write(const Point_ref &tri) { return callback_(tri); }
  bool write(float *a) {
    return write(Point_ref{
        reinterpret_cast<glm::vec3 *>(a),
    });
  }
  std::function<bool(const Point_ref &tri)> callback_{}; // return false to
                                                         // break.
};
} // namespace data_stream
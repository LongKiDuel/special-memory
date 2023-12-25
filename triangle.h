#pragma once

#include <array>
#include <functional>
#include <glm/ext/vector_float3.hpp>
namespace data_stream {
struct Triangle_ref {
  std::array<glm::vec3 *, 3> points_;
};
struct Triangle_stream {
  bool write(const Triangle_ref &tri) { return callback_(tri); }
  bool write(float *a, float *b, float *c) {
    return write(Triangle_ref{
        reinterpret_cast<glm::vec3 *>(a),
        reinterpret_cast<glm::vec3 *>(b),
        reinterpret_cast<glm::vec3 *>(c),
    });
  }
  std::function<bool(const Triangle_ref &tri)> callback_{}; // return false to
                                                            // break.
};
} // namespace data_stream
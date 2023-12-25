#pragma once
#include "point.h"
#include "triangle.h"
namespace data_stream {
inline void rebind(Triangle_stream &from, Point_stream &to) {
  from.callback_ = [&to](const Triangle_ref &ref) {
    for (auto p : ref.points_) {
      if (!to.write({p})) {
        return false;
      }
    }
    return true;
  };
}
} // namespace data_stream
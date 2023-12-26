#pragma once
#include <functional>
#include <vector>
namespace ImGuiX {
class Font_build_queue {
public:
  std::vector<std::function<void()>> queue_;
};
namespace details {
inline Font_build_queue build_queue;
}
} // namespace ImGuiX
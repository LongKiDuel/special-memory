#pragma once

#include <cstdint>
#include <imgui.h>
#include <memory>
#include <string>
#include <vector>
namespace ImGuiX {
inline ImFontConfig default_font_config() {
  ImFontConfig config{};
  config.PixelSnapH = true;
  config.OversampleH = 1;
  config.OversampleV = 1;
  return config;
}
struct Font_info {
  uint32_t size_{};
  std::string file_path_{};
  ImFontConfig config_{};
  std::vector<ImWchar> ranges_{};

  auto operator==(const Font_info &i) const {
    return size_ == i.size_ && file_path_ == i.file_path_ &&
           ranges_ == i.ranges_;
  }
};
struct Font_data {
  Font_info info_{};
  ImFont *font_{};
};
class Font_manager {
public:
  ImFont *lookup(Font_info info) {
    for (auto &f : fonts_) {
      if (f.info_ == info) {
        return f.font_;
      }
    }
    return nullptr;
  }
  auto raii_push(Font_info info) {
    struct Pop_font {
      bool valid_{};
      Pop_font() {}
      Pop_font(Pop_font &&rhs) : valid_(rhs.valid_) { rhs.valid_ = false; }
      ~Pop_font() {
        if (valid_) {
          ImGui::PopFont();
        }
      }
    };
    Pop_font pop;
    for (auto &f : fonts_) {
      if (f.info_ == info) {
        ImGui::PushFont(f.font_);
        pop.valid_ = true;
        break;
      }
    }
    return pop;
  }
  bool build(const Font_info &info) {
    if (lookup(info)) {
      return true;
    }

    auto &io = ImGui::GetIO();
    std::unique_ptr<std::vector<ImWchar>> ranges_storage;
    if (!info.ranges_.empty()) {
      ranges_storage = std::make_unique<std::vector<ImWchar>>(info.ranges_);
    }
    auto font = io.Fonts->AddFontFromFileTTF(
        info.file_path_.c_str(), info.size_, &info.config_,
        info.ranges_.empty() ? nullptr : ranges_storage->data());

    Font_data data;
    data.font_ = font;
    data.info_ = info;
    fonts_.push_back(data);
    if (ranges_storage) {
      ranges_.push_back(std::move(ranges_storage));
    }
    return font;
  }

private:
  std::vector<Font_data> fonts_{};
  std::vector<std::unique_ptr<std::vector<ImWchar>>> ranges_{};
};
} // namespace ImGuiX
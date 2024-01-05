#pragma once
#include "bitmap.h"
#include <memory>
#include <optional>
#include <string>
#include <vector>
namespace image_mix {
class Loader {
public:
  virtual ~Loader() = default;
  virtual std::optional<Bitmap> load(const std::string &file_path) = 0;
  virtual std::string name() const = 0;
};

class Loader_set : public Loader {
public:
  std::optional<Bitmap> load(const std::string &file_path) {
    for (auto &loader : loaders_) {
      if (auto bitmap = loader->load(file_path)) {
        return bitmap;
      }
    }
    return {};
  }

  void add_load(std::unique_ptr<Loader> &&loader) {
    loaders_.push_back(std::move(loader));
  }

  virtual std::string name() const { return "Loader set"; }

private:
  std::vector<std::unique_ptr<Loader>> loaders_;
};
} // namespace image_mix
#include "image_loader.h"
#include "loader_stb.h"
#include <memory>

std::unique_ptr<image_mix::Loader> image_mix::get_default_loader() {
  auto loader = std::make_unique<Loader_set>();
  loader->add_load(std::make_unique<Loader_stb>());
  return loader;
}

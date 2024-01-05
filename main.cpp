#include "image_loader.h"
#include <fmt/core.h>
#include <iostream>
#include <string>
int main(int argc, char **argv) {
  if (argc < 2) {
    std::cout << "Useage: " << argv[0] << " <File path>"
              << "\n";
    return 1;
  }
  std::string file = argv[1];

  image_mix::Loader_set loader;

  auto bitmap = loader.load(file);
  if (!bitmap) {
    std::cout << "Failed to load bitmap from: " << file << "\n";
    return 2;
  }

  std::cout << file << ":\n";
  std::cout << fmt::format("width: {} height: {} channel: {}\n",
                           bitmap->width(), bitmap->height(),
                           bitmap->channel());
}

#include "image_exporter_stb.h"
#include "image_exporter_webp.h"
#include "image_loader.h"
#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <string>
int main(int argc, char **argv) {
  if (argc < 2) {
    std::cout << "Useage: " << argv[0] << " <File path>"
              << "\n";
    return 1;
  }
  std::string file = argv[1];

  auto loader = image_mix::get_default_loader();

  auto bitmap = loader->load(file);
  if (!bitmap) {
    std::cout << "Failed to load bitmap from: " << file << "\n";
    return 2;
  }

  std::cout << file << ":\n";
  std::cout << fmt::format("width: {} height: {} channel: {}\n",
                           bitmap->width(), bitmap->height(),
                           bitmap->channel());

  image_mix::Exporter_stb_jpg exporter;
  std::string output_file_name =
      std::string{"output"} + exporter.common_extension();
  std::ofstream output{output_file_name, std::ios::binary};
  auto image_encoded = exporter.export_to_buffer(*bitmap).value();
  output.write(reinterpret_cast<char *>(image_encoded.data()),
               image_encoded.size());
}

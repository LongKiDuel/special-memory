
#include "compress_context.h"
int main() {
  auto compress = create_zstd_compress_stream();

  compress->compress(std::span{"abc"});
}
#include "compress_context.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <zstd.h>
class Compress_stream_context_zstd : public Compress_stream_context {
public:
  Compress_stream_context_zstd(int compress_level) {
    cstream = ZSTD_createCStream();
    if (cstream == nullptr) {
      throw std::runtime_error("Failed to create compression stream");
    }
    ZSTD_initCStream(cstream, compress_level);
  }

  ~Compress_stream_context_zstd() override {
    finish_frame();
    ZSTD_freeCStream(cstream);
  }

  void compress(std::span<const char> data) override {
    ZSTD_inBuffer input = {data.data(), data.size(), 0};
    std::vector<char> buffer(ZSTD_CStreamOutSize());

    while (input.pos < input.size) {
      auto output = create_out_buffer();
      size_t const code =
          ZSTD_compressStream2(cstream, &output, &input, ZSTD_e_continue);
      if (ZSTD_isError(code)) {
        throw std::runtime_error(ZSTD_getErrorName(code));
      }

      call_write(output);
    }
  }

  void write_function(std::span<const char> write_data) override {
    // Example implementation: Write compressed data to stdout
    std::cout.write(write_data.data(), write_data.size());
  }

  void flush_block() {}
  void finish_frame() {
    while (true) {
      auto input = create_empty_input();
      auto output = create_out_buffer();
      auto code = ZSTD_compressStream2(cstream, &output, &input, ZSTD_e_end);
      if (ZSTD_isError(code)) {
        throw std::runtime_error(ZSTD_getErrorName(code));
      }
      call_write(output);
      if (code == 0) {
        break;
      }
    }
  }

  void reset() override {
    finish_frame();
    ZSTD_CCtx_reset(cstream, ZSTD_reset_session_only);
  }

private:
  ZSTD_outBuffer create_out_buffer() {
    ZSTD_outBuffer output = {buffer.data(), buffer.size(), 0};
    return output;
  }
  void call_write(ZSTD_outBuffer &output) {
    if (output.pos > 0) {
      write_function({(const char *)output.dst, output.pos});
    }
  }
  ZSTD_inBuffer create_empty_input() {
    ZSTD_inBuffer buffer{};
    return buffer;
  }

private:
  ZSTD_CStream *cstream;
  std::vector<char> buffer = std::vector<char>(ZSTD_CStreamOutSize());
};

std::unique_ptr<Compress_stream_context>
create_zstd_compress_stream(int level) {
  return std::make_unique<Compress_stream_context_zstd>(level);
}
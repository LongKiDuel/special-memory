#include <cassert>
#include <cstdint>
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/system.h>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
namespace curlpp::function_types {
using progress_callback = int(void *user_data, curl_off_t download_total,
                              curl_off_t download_current,
                              curl_off_t upload_total,
                              curl_off_t upload_current);
}
namespace curlpp::built_in_functions {
int cout_progress_bar(void *user_data, curl_off_t download_total,
                      curl_off_t download_current, curl_off_t upload_total,
                      curl_off_t upload_current) {
  (void)user_data;
  auto print = [](auto name, auto value) {
    std::cout << name << " " << value << " ";
  };
  print("dl total", download_total);
  print("dl current", download_current);
  print("ul total", upload_total);
  print("ul current", upload_current);
  std::cout << "\n";

  return 0;
}
} // namespace curlpp::built_in_functions
namespace curlpp {

class Easy_handle {
public:
  Easy_handle() { curl_ = curl_easy_init(); }
  Easy_handle(Easy_handle &&handle) {
    curl_ = handle.curl_;
    handle.curl_ = curl_easy_init();
  }
  ~Easy_handle() { curl_easy_cleanup(curl_); }
  operator CURL *() { return curl_; }
  auto set_opt(CURLoption option, auto value) {
    return curl_easy_setopt(curl_, option, value);
  }

private:
  CURL *curl_;
};
class Mime_handle {
public:
  Mime_handle() { mime_ = curl_mime_init(nullptr); }
  ~Mime_handle() { curl_mime_free(mime_); }
  Mime_handle(Mime_handle &&rhs) {
    mime_ = rhs.mime_;
    rhs.mime_ = curl_mime_init(nullptr);
  }
  Mime_handle &operator=(Mime_handle &&rhs) {
    curl_mime_free(mime_);
    mime_ = rhs.mime_;
    rhs.mime_ = curl_mime_init(nullptr);

    return *this;
  }
  operator curl_mime *() { return mime_; };

  // key and value will be copied into inner buffer.
  void add_string(const std::string &key, const std::string &value) {
    auto part = curl_mime_addpart(mime_);

    curl_mime_name(part, key.c_str());
    curl_mime_data(part, value.c_str(), value.size());
  }

  // key, file name and buffer will be copied into inner buffer.
  void add_file_from_memory(const std::string &key, const std::string &filename,
                            std::vector<char> buffer) {
    auto part = curl_mime_addpart(mime_);

    curl_mime_name(part, key.c_str());
    curl_mime_filename(part, filename.c_str());
    curl_mime_data(part, buffer.data(), buffer.size());
  }

  void add_file(const std::string &key, const std::string &filename,
                const std::string &path_for_read_file) {
    auto part = curl_mime_addpart(mime_);

    curl_mime_name(part, key.c_str());
    curl_mime_filename(part, filename.c_str());
    curl_mime_filedata(part, path_for_read_file.c_str());
  }

private:
  curl_mime *mime_{};
};
class Extened_easy_handle : public Easy_handle {
public:
  Extened_easy_handle() = default;
  Extened_easy_handle(Extened_easy_handle &&) = delete;
  Extened_easy_handle(const Extened_easy_handle &) = delete;

  void set_url(const std::string &url) { set_opt(CURLOPT_URL, url.c_str()); }
  void set_verbose(bool verbose) {
    long value = verbose;

    set_opt(CURLOPT_VERBOSE, 1L);
  }

  void
  set_progress_bar(std::function<function_types::progress_callback> callback) {
    progress_callback_call_ = callback;
    set_opt(CURLOPT_XFERINFOFUNCTION, progress_bar_adoptor);
    set_opt(CURLOPT_XFERINFODATA, this);
    set_opt(CURLOPT_NOPROGRESS, 0);
  }

  void add_mime(Mime_handle mime) {
    mime_ = std::make_unique<Mime_handle>(std::move(mime));
    set_opt(CURLOPT_MIMEPOST, (curl_mime *)(*mime_));
  }

  void change_to_post() { set_opt(CURLOPT_POST, 1L); }
  // proxy all in one
  void set_proxy(const std::string &proxy) {
    auto result = set_opt(CURLOPT_PROXY, proxy.c_str());
    assert(result == CURLE_OK);
  }
  // proxy per-part
  void set_proxy_host(const std::string &host) {
    set_opt(CURLOPT_PROXY, host.c_str());
  }
  void set_proxy_type(curl_proxytype type) { set_opt(CURLOPT_PROXYTYPE, type); }
  void set_proxy_port(uint16_t port) {
    long value = port;
    set_opt(CURLOPT_PROXYPORT, value);HTTP proxies can generally only speak HTTP (for obvious reasons), which makes libcurl convert non-HTTP requests to HTTP when using an HTTP proxy without this tunnel option set. For example, asking for an FTP URL and specif
  }

private:
  static int progress_bar_adoptor(void *body, curl_off_t a, curl_off_t b,
                                  curl_off_t c, curl_off_t d) {
    auto self = reinterpret_cast<Extened_easy_handle *>(body);
    return self->progress_callback_call_(body, a, b, c, d);
  }

private:
  std::function<function_types::progress_callback> progress_callback_call_;
  std::unique_ptr<Mime_handle> mime_;
};

} // namespace curlpp

int main() {
  curlpp::Extened_easy_handle handle{};

  handle.set_verbose(true);
  handle.set_url("http://localhost:9812");
  handle.set_progress_bar(curlpp::built_in_functions::cout_progress_bar);

  curlpp::Mime_handle mime;
  mime.add_string("name", "Bob");
  mime.add_string("time", "today");
  mime.add_file("program", "program", "main");

  handle.add_mime(std::move(mime));

  handle.set_opt(CURLOPT_NOPROXY, "localhost");

  curl_easy_perform(handle);
}

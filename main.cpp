#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/system.h>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
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
    std::cout << name << " " << value << "\n";
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
    curl_easy_setopt(curl_, option, value);
  }

private:
  CURL *curl_;
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

private:
  static int progress_bar_adoptor(void *body, curl_off_t a, curl_off_t b,
                                  curl_off_t c, curl_off_t d) {
    auto self = reinterpret_cast<Extened_easy_handle *>(body);
    return self->progress_callback_call_(body, a, b, c, d);
  }

private:
  std::function<function_types::progress_callback> progress_callback_call_;
};
} // namespace curlpp

int main() {
  curlpp::Extened_easy_handle handle{};

  handle.set_verbose(true);
  handle.set_url("https://www.bing.com");
  handle.set_progress_bar(curlpp::built_in_functions::cout_progress_bar);

  curl_easy_perform(handle);
}

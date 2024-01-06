#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/system.h>
#include <curl/urlapi.h>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
namespace curlpp::function_types {
using progress_callback = int(void *user_data, curl_off_t download_total,
                              curl_off_t download_current,
                              curl_off_t upload_total,
                              curl_off_t upload_current);

using write_fucnction = size_t(char *buffer, size_t size);
using header_fucnction = size_t(char *buffer, size_t size);
} // namespace curlpp::function_types
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

class Header_list {
public:
  Header_list() { list_ = nullptr; }
  ~Header_list() { curl_slist_free_all(list_); }

  Header_list(Header_list &&rhs) {
    list_ = rhs.list_;
    rhs.list_ = nullptr;
  }
  Header_list &operator=(Header_list &&rhs) {
    list_ = rhs.list_;
    rhs.list_ = {};

    return *this;
  }
  operator curl_slist *() { return list_; };

  void add_string(const std::string &str) {
    list_ = curl_slist_append(list_, str.c_str());
  }
  void add(const std::string &name, const std::string &value) {
    add_string(name + ": " + value);
  }

private:
  curl_slist *list_{};
};

class Extened_easy_handle : public Easy_handle {
public:
  Extened_easy_handle() = default;
  Extened_easy_handle(Extened_easy_handle &&) = delete;
  Extened_easy_handle(const Extened_easy_handle &) = delete;

  void set_url(const std::string &url) { set_opt(CURLOPT_URL, url.c_str()); }
  void set_verbose(bool verbose) {
    long value = verbose;

    set_opt(CURLOPT_VERBOSE, value);
  }

  void
  set_progress_bar(std::function<function_types::progress_callback> callback) {
    progress_callback_call_ = callback;
    set_opt(CURLOPT_XFERINFOFUNCTION, progress_bar_adoptor);
    set_opt(CURLOPT_XFERINFODATA, this);
    set_opt(CURLOPT_NOPROGRESS, 0);
  }
  void set_write_function(
      std::function<function_types::write_fucnction> write_callback) {
    write_callback_call_ = std::move(write_callback);
    set_opt(CURLOPT_WRITEDATA, this);
    set_opt(CURLOPT_WRITEFUNCTION, write_function_adoptor);
  }

  // called per-line before http body.
  void set_header_function(
      std::function<function_types::write_fucnction> header_callback) {
    header_callback_call_ = std::move(header_callback);
    set_opt(CURLOPT_HEADERDATA, this);
    set_opt(CURLOPT_HEADERFUNCTION, header_function_adoptor);
  }

  void add_mime(Mime_handle mime) {
    mime_ = std::make_unique<Mime_handle>(std::move(mime));
    set_opt(CURLOPT_MIMEPOST, (curl_mime *)(*mime_));
  }
  void add_headers(Header_list header_list) {
    headers_ = std::make_unique<Header_list>(std::move(header_list));
    set_opt(CURLOPT_HTTPHEADER, static_cast<curl_slist *>(*headers_));
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
    set_opt(CURLOPT_PROXYPORT, value);
  }

  void perform() { curl_easy_perform(*this); }

  void user_auth(const std::string &name, const std::string &password) {
    std::string str = name + ":" + password;
    set_opt(CURLOPT_USERPWD, str.c_str());
  }

private:
  static int progress_bar_adoptor(void *body, curl_off_t a, curl_off_t b,
                                  curl_off_t c, curl_off_t d) {
    auto self = reinterpret_cast<Extened_easy_handle *>(body);
    return self->progress_callback_call_(body, a, b, c, d);
  }

  static int write_function_adoptor(char *ptr, size_t size, size_t nmemb,
                                    void *userdata) {
    auto self = reinterpret_cast<Extened_easy_handle *>(userdata);
    // size is always 1 in the document.
    assert(size == 1);
    return self->write_callback_call_(ptr, nmemb);
  }
  static size_t header_function_adoptor(char *buffer, size_t size,
                                        size_t nitems, void *userdata) {
    auto self = reinterpret_cast<Extened_easy_handle *>(userdata);
    // size is always 1 in the document.
    assert(size == 1);
    return self->header_callback_call_(buffer, nitems);
  }

private:
  std::function<function_types::progress_callback> progress_callback_call_;
  std::function<function_types::write_fucnction> write_callback_call_;
  std::function<function_types::header_fucnction> header_callback_call_;
  std::unique_ptr<Mime_handle> mime_;
  std::unique_ptr<Header_list> headers_;
};

std::string get_curl_version() { return curl_version(); }

class Url_handle {
public:
  Url_handle() {
    handle_ = curl_url();
    assert(handle_);
  }
  CURLU *handle_{};
  // if failed, return error info.
  std::optional<std::string> parse(const std::string &url) {
    auto rc = curl_url_set(handle_, CURLUPART_URL, url.c_str(), 0);
    if (rc) {
      return curl_url_strerror(rc);
    }
    return {};
  }
  std::optional<std::string> get_path() {
    char *path{};
    auto rc = curl_url_get(handle_, CURLUPART_PATH, &path, 0);

    std::optional<std::string> result;
    if (!rc) {
      result = path;
    }
    if (path) {
      curl_free(path);
    }
    return result;
  }
  ~Url_handle() {
    curl_url_cleanup(handle_);
    handle_ = nullptr;
  }
};
} // namespace curlpp

void url_test() {
  auto try_url = [](std::string url) {
    curlpp::Url_handle handle;
    auto err = handle.parse(url);
    if (err) {
      std::cerr << "Failed to parse: " << url << " -> " << err.value() << "\n";
    }
    assert(!err);
    std::cout << handle.get_path().value() << "\n";
  };

  try_url("file:///abc/123");
  try_url("http://tome.com/source?name=ro");
}
int main() {
  url_test();

  curlpp::Extened_easy_handle handle{};

  handle.set_verbose(true);
  handle.set_url("http://localhost:9812");
  handle.set_progress_bar(curlpp::built_in_functions::cout_progress_bar);

  curlpp::Mime_handle mime;
  mime.add_string("name", "Bob");
  mime.add_string("time", "today");

  handle.add_mime(std::move(mime));

  curlpp::Header_list headers;

  headers.add("X-name", "Alice");

  handle.add_headers(std::move(headers));
  std::string buffer;
  handle.set_write_function([&buffer](auto buf, auto size) {
    buffer.append(buf, size);
    return size;
  });
  handle.set_verbose(false);

  handle.set_header_function([i = 0](auto buf, auto size) mutable {
    std::cout << i++ << " " << std::string_view(buf, size) << "\n";
    return size;
  });

  handle.set_opt(CURLOPT_NOPROXY, "localhost");
  handle.user_auth("Bobo", "F1 89sf");

  handle.perform();

  std::cout << buffer << "\n";
}

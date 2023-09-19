#include "PutFile.h"
#include <drogon/HttpResponse.h>
#include <filesystem>
#include <fstream>
#include <ios>
#include <string>
#include <trantor/utils/Logger.h>

namespace {
void print_req(auto req) {
  std::cout << req->path() << "\n";
  for (auto [k, v] : req->headers()) {
    std::cout << "Header: " << k << ": " << v << "\n";
  }
  // std::cout << req->body() << "\n";
}
auto extrac_path(std::string p) {
  auto path = p.substr(1);
  auto sub_path = path.substr(path.find('/') + 1);
  return sub_path;
}
auto get_file(std::string url_path) {
  std::filesystem::path root = "/tmp/vc-cache/";
  std::cout << "match at " << url_path << "\n";
  auto p = extrac_path(url_path);

  return root / p;
}
} // namespace

void PutFiles::get(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) const {
  auto file = get_file(req->path());
  if(std::filesystem::exists(file)){
    callback(HttpResponse::newFileResponse(file));
    return;
  }
  callback(HttpResponse::newNotFoundResponse());
}
void PutFiles::put(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) const {
  auto c = HttpResponse::newHttpResponse();
  c->setBody(std::string(req->body()));
  print_req(req);

  auto p = get_file(req->path());
  std::cout << "position: " << p << "\n";
  if (!std::filesystem::exists(p.parent_path())) {
    std::filesystem::create_directories(p.parent_path());
  }
  std::fstream out{p, std::ios::binary | std::ios::out};
  out.write(req->bodyData(), req->bodyLength());
  req->getBody();

  callback(c);
}
void PutFiles::check(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) const {
  std::cout << "HEAD!\n";
  auto c = HttpResponse::newHttpResponse();
  c->setBody("CHECK");
  print_req(req);

  callback(c);
}

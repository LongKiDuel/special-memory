#include "Cmake.h"
#include <cstdlib>
#include <drogon/HttpResponse.h>
#include <drogon/utils/FunctionTraits.h>
#include <format>
#include <functional>
namespace {
std::string executeCommand(
    const char *cmd,
    std::function<void(std::string)> stream_callback = [](auto) {}) {
  std::array<char, 128> buffer;
  std::string result;

  // Open a pipe to the command
  FILE *pipe = popen(cmd, "r");
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }

  // Read the command's output into the result string
  while (!feof(pipe)) {
    if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
      result += buffer.data();
      stream_callback(buffer.data());
    }
  }

  // Close the pipe and return the result
  pclose(pipe);
  return result;
}
} // namespace
void api::v1::CMake::build(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {
  auto p = req->getParameter("path");
  if (p.empty()) {
    callback(HttpResponse::newNotFoundResponse());
    return;
  }
  auto cmd = std::format(
      "cmake -B {}/build -S {} && cmake --build {}/build 2>&1", p, p, p, p);
  std::cout << cmd << "\n";

  auto r = HttpResponse::newHttpResponse();
  r->setBody("id:1");
  r->addHeader("Refresh", "1;url=build-log?id=1"); // url=xxx
  r->setContentTypeCode(ContentType::CT_TEXT_PLAIN);
  callback(r);
  finished_ = false;
  buffer_.clear();
  std::thread t {
    [=,this] {
      auto result = executeCommand(
          cmd.c_str(), [this](auto stream_input) { buffer_ += stream_input; });
      finished_ = true;
    }
  };
  t.detach();
}
void api::v1::CMake::build_log(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) const {
  auto r = HttpResponse::newHttpResponse();
  auto id = req->getParameter("id");
  r->setBody(buffer_);
  if (!finished_) {
    r->addHeader("Refresh", "1;"); // url=xxx
  }
  r->setContentTypeCode(ContentType::CT_TEXT_PLAIN);
  callback(r);
}
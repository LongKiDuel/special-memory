#include "Cmake.h"
#include <cstdlib>
#include <drogon/utils/FunctionTraits.h>
#include <format>
namespace {
std::string executeCommand(const char *cmd) {
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
    }
  }

  // Close the pipe and return the result
  pclose(pipe);
  return result;
}
} // namespace
void api::v1::CMake::build(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) const {
  auto p = req->getParameter("path");
  if (p.empty()) {
    callback(HttpResponse::newNotFoundResponse());
    return;
  }
  auto cmd = std::format("cmake -B {}/build -S {} && cmake --build {}/build 2>&1", p,
                         p, p, p);
  std::cout << cmd << "\n";
  auto result = executeCommand(cmd.c_str());
  auto r = HttpResponse::newHttpResponse();
  r->setBody(result);
  r->setContentTypeCode(ContentType::CT_TEXT_PLAIN);
  callback(r);
}

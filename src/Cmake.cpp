#include "Cmake.h"
#include <cstdint>
#include <cstdlib>
#include <drogon/HttpResponse.h>
#include <drogon/utils/FunctionTraits.h>
#include <format>
#include <functional>
#include <string>
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
  uint64_t id{};
  Build_task *task{};

  auto lock = next_task(p, &id, &task);
  if (!lock) {
    callback(HttpResponse::newRedirectionResponse(
        std::format("build-log?id={}", id)));
    return;
  }

  auto r = HttpResponse::newHttpResponse();
  r->setBody(std::format("id:{}", id));
  r->addHeader("Refresh", std::format("1;url=build-log?id={}", id)); // url=xxx
  r->setContentTypeCode(ContentType::CT_TEXT_PLAIN);
  callback(r);

  std::thread t{[cmd, id, task, lock] {
    auto result = executeCommand(cmd.c_str(), [task](auto stream_input) {
      task->append_buffer(stream_input);
    });
    task->finished_ = true;
  }};
  t.detach();
}
void api::v1::CMake::build_log(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) const {
  auto r = HttpResponse::newHttpResponse();
  auto id = std::stoi(req->getParameter("id"));
  if (!tasks_.count(id)) {
    callback(HttpResponse::newNotFoundResponse());
    return;
  }
  auto &task = tasks_.find(id)->second;
  r->setBody(task.read_buffer());
  if (!task.finished_) {
    r->addHeader("Refresh", "1;"); // url=xxx
  }
  r->setContentTypeCode(ContentType::CT_TEXT_PLAIN);
  callback(r);
}
std::optional<std::any>
api::v1::CMake::next_task(std::string path, uint64_t *id, Build_task **task) {
  auto obj = lock_task(path);
  if (!obj) {
    *id = locks_[path].id;
    return {};
  }
  auto i = tasks_.size();
  *id = i;
  auto &t = tasks_[i];
  *task = &t;
  return std::move(obj);
}

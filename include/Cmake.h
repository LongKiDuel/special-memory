/// The header file
#pragma once
#include <any>
#include <clocale>
#include <cstdint>
#include <drogon/HttpController.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <format>
#include <json/value.h>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
using namespace drogon;
namespace api {
namespace v1 {
class CMake : public drogon::HttpController<CMake> {
public:
  METHOD_LIST_BEGIN
  // use METHOD_ADD to add your custom processing function here;
  METHOD_ADD(CMake::build, "/build",
             {Get, Post, Options}); // path is /api/v1/CMake/build
  METHOD_ADD(CMake::build_log, "/build-log",
             {Get, Post, Options}); // path is /api/v1/CMake/build
  METHOD_LIST_END
  // your declaration of processing function maybe like this:
  void build(const HttpRequestPtr &req,
             std::function<void(const HttpResponsePtr &)> &&callback);
  void build_log(const HttpRequestPtr &req,
                 std::function<void(const HttpResponsePtr &)> &&callback) const;

private:
  struct Build_task {
    std::string path_;
    bool finished_{};
    std::string read_buffer() const {
      std::unique_lock lock{m_};
      return output_buffer_;
    }
    void append_buffer(std::string_view s) {
      std::unique_lock lock{m_};
      output_buffer_ += s;
    }

  private:
    std::string output_buffer_;
    mutable std::mutex m_;
  };
  std::unordered_map<uint64_t, Build_task> tasks_;
  struct Task_lock {
    uint64_t id{};
  };

  std::unordered_map<std::string, Task_lock> locks_;
  std::optional<std::any> lock_task(std::string path) {
    if (locks_.count(path)) {
      return {};
    }
    locks_[path] = Task_lock{tasks_.size()};
    struct RAII_lock {
      CMake *origin{};
      std::string release_target;
      RAII_lock(CMake *o, std::string t) {
        origin = o;
        release_target = t;
        std::cout << std::format("create lock on {}\n", release_target);
      }
      RAII_lock(const RAII_lock &) = delete;
      RAII_lock(RAII_lock &&rhs) {
        origin = rhs.origin;
        release_target = rhs.release_target;
        rhs.origin = nullptr;
      }
      ~RAII_lock() {
        if (origin) {
          std::cout << std::format("release lock on {}\n", release_target);
          origin->release_task_lock(release_target);
        }
      }
    };
    std::any obj = std::make_shared<RAII_lock>(RAII_lock{this, path});
    return obj;
  }
  void release_task_lock(std::string path) { locks_.erase(path); }
  // return hold locks
  std::optional<std::any> next_task(std::string path, uint64_t *id,
                                    Build_task **task);
};
} // namespace v1
} // namespace api
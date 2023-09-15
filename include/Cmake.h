/// The header file
#pragma once
#include <drogon/HttpController.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <json/value.h>
#include <string>
using namespace drogon;
namespace api {
namespace v1 {
class CMake : public drogon::HttpController<CMake> {
public:
  METHOD_LIST_BEGIN
  // use METHOD_ADD to add your custom processing function here;
  METHOD_ADD(CMake::build, "/build",{Get,Post,Options}); // path is /api/v1/CMake/build
  METHOD_ADD(CMake::build_log, "/build-log",{Get,Post,Options}); // path is /api/v1/CMake/build
  METHOD_LIST_END
  // your declaration of processing function maybe like this:
  void build(const HttpRequestPtr &req,
             std::function<void(const HttpResponsePtr &)> &&callback)  ;
  void build_log(const HttpRequestPtr &req,
                 std::function<void(const HttpResponsePtr &)> &&callback) const;

private:
std::string buffer_;
bool finished_{};
};
} // namespace v1
} // namespace api
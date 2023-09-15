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
class Health : public drogon::HttpController<Health> {
public:
  METHOD_LIST_BEGIN
  // use METHOD_ADD to add your custom processing function here;
  METHOD_ADD(Health::is_ok, "/ok",{Get,Post,Options}); // path is /api/v1/Health/ping
  METHOD_LIST_END
  // your declaration of processing function maybe like this:
  void is_ok(const HttpRequestPtr &req,
             std::function<void(const HttpResponsePtr &)> &&callback) const;

public:
};
} // namespace v1
} // namespace api
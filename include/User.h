/// The header file
#pragma once
#include <drogon/HttpController.h>
#include <drogon/HttpResponse.h>
#include <json/value.h>
using namespace drogon;
namespace api {
namespace v1 {
class User : public drogon::HttpController<User> {
public:
  METHOD_LIST_BEGIN
  // use METHOD_ADD to add your custom processing function here;
  METHOD_ADD(User::getInfo, "/{id}", Get); // path is /api/v1/User/{arg1}
  METHOD_LIST_END
  // your declaration of processing function maybe like this:
  void getInfo(const HttpRequestPtr &req,
               std::function<void(const HttpResponsePtr &)> &&callback,
               int userId) const;

public:
  User() { LOG_DEBUG << "User constructor!"; }
};
} // namespace v1
} // namespace api
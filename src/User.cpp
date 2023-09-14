#include "User.h"
void api::v1::User::getInfo(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback, int userId) const {
  Json::Value v;
  v["id"] = userId;
  callback(HttpResponse::newHttpJsonResponse(v));
}

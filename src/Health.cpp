#include "Health.h"
#include <drogon/HttpResponse.h>
#include <drogon/utils/FunctionTraits.h>

namespace {

HttpResponsePtr ok() {
  auto response = HttpResponse::newHttpResponse();
  response->setBody("ok");
  return response;
}
} // namespace

void api::v1::Health::is_ok(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) const {
  callback(ok());
}

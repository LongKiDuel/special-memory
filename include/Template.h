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
class XXXF : public drogon::HttpController<XXXF> {
public:
  METHOD_LIST_BEGIN
  // use METHOD_ADD to add your custom processing function here;
  METHOD_ADD(XXXF::getInfo, "/{1}",{Get,Post,Options}); // path is /api/v1/XXXF/{arg1}
  METHOD_LIST_END
  // your declaration of processing function maybe like this:
  void getInfo(const HttpRequestPtr &req,
               std::function<void(const HttpResponsePtr &)> &&callback) const; 
public:
};
} // namespace v1
} // namespace api
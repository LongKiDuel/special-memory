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
class Files : public drogon::HttpController<Files> {
public:
  METHOD_LIST_BEGIN
  // use METHOD_ADD to add your custom processing function here;
  METHOD_ADD(Files::getInfo, "/getInfo",{Get,Post,Options}); // path is /api/v1/User/{arg1}
  METHOD_ADD(Files::getFetch, "/fetch",{Get,Post,Options}); 
  METHOD_LIST_END
  // your declaration of processing function maybe like this:
  void getInfo(const HttpRequestPtr &req,
               std::function<void(const HttpResponsePtr &)> &&callback) const;
  void getFetch(const HttpRequestPtr &req,
                std::function<void(const HttpResponsePtr &)> &&callback) const;

public:
};
} // namespace v1
} // namespace api
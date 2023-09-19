/// The header file
#pragma once
#include <drogon/HttpController.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <json/value.h>
#include <string>
using namespace drogon;
 
class PutFiles : public drogon::HttpController<PutFiles> {
public:
  METHOD_LIST_BEGIN
  // use METHOD_ADD to add your custom processing function here;
  ADD_METHOD_VIA_REGEX(PutFiles::put, "/vcpkg-cache/.*",{Put}); // path is /api/v1/User/{arg1}
  ADD_METHOD_VIA_REGEX(PutFiles::get, "/vcpkg-cache/.*",{Get}); // path is /api/v1/User/{arg1}
  ADD_METHOD_VIA_REGEX(PutFiles::check, "/vcpkg-cache/.*",{Head}); // path is /api/v1/User/{arg1}
  METHOD_LIST_END
  // your declaration of processing function maybe like this:
  void put(const HttpRequestPtr &req,
           std::function<void(const HttpResponsePtr &)> &&callback) const;
  void get(const HttpRequestPtr &req,
           std::function<void(const HttpResponsePtr &)> &&callback) const;
  void check(const HttpRequestPtr &req,
             std::function<void(const HttpResponsePtr &)> &&callback) const;

public:
};
// http,http://localhost:9812/vcpkg-cache/{name}/{version}/{sha},readwrite
// export VCPKG_BINARY_SOURCES="http,http://localhost:9812/vcpkg-cache/{name}/{version}/{sha},readwrite"
//  --binarysource="http,http://localhost:9812/vcpkg-cache/{name}/{version}/{sha},readwrite"

// vcpkg install --binarysource="http,http://localhost:9812/vcpkg-cache/{name}/{version}/{sha},readwrite" imgui
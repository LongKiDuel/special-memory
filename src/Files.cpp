#include "Files.h"
#include <chrono>
#include <cstring>
#include <ctime>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/utils/FunctionTraits.h>
#include <filesystem>
#include <json/reader.h>
#include <json/value.h>
#include <optional>
#include <ostream>
#include <string>

struct File_info {
  std::string type;
  std::string name;
  std::string size;
  std::string write_time;
};

File_info read_file_info(const std::filesystem::path &p) {
  File_info info;

  // Check if file exists
  if (!std::filesystem::exists(p)) {
    throw std::runtime_error("Path does not exist");
  }

  // Type
  if (std::filesystem::is_regular_file(p)) {
    info.type = "Regular File";
  } else if (std::filesystem::is_directory(p)) {
    info.type = "Directory";
  } else {
    info.type = "Other";
  }

  // Name
  info.name = p.filename().string();

  // Size (only for regular files)
  if (std::filesystem::is_regular_file(p)) {
    info.size = std::to_string(std::filesystem::file_size(p)) + " bytes";
  } else {
    info.size = "N/A";
  }

  // Function to convert time to human-readable format
  auto time_to_string = [](time_t file_time) {
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&file_time), "%Y-%m-%d %H:%M:%S");
    // ss << std::put_time(std::localtime(&file_time), "%Y-%m-%d %H:%M:%S");
    // ss << file_time;
    return ss.str();
  };

  // Write time
  auto t = std::filesystem::last_write_time(p);
  auto sys_t = std::filesystem::file_time_type::clock::to_sys(t);

  info.write_time = time_to_string(
      std::chrono::duration_cast<std::chrono::seconds>(sys_t.time_since_epoch())
          .count());

  return info;
}

std::optional<std::string> generate_weak_etag(std::filesystem::path file) {
  if (std::filesystem::is_regular_file(file)) {
    return std::to_string(
        std::filesystem::last_write_time(file).time_since_epoch().count());
  }
  return {};
}

Json::Value file_info_to_json(const File_info &info) {
  Json::Value json;

  json["type"] = info.type;
  json["name"] = info.name;
  json["size"] = info.size;
  json["write_time"] = info.write_time;

  return json;
}

void allow_cors(auto &response) {
  response->addHeader("Access-Control-Allow-Origin", "*");
}

void api::v1::Files::getInfo(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) const {
  auto jsonPtr = req->jsonObject();
  auto reportFailure = [&] {
    auto resp = HttpResponse::newNotFoundResponse();
    resp->addHeader("Access-Control-Allow-Origin", "*");

    callback(resp);
  };
  if (!jsonPtr) {
    reportFailure();
    return;
  }
  auto path = (*jsonPtr)["path"].asString();
  if (!std::filesystem::exists(path)) {
    reportFailure();
    return;
  }

  HttpResponsePtr response;
  if (std::filesystem::is_directory(path)) {
    std::filesystem::directory_iterator begin{path}, end;
    Json::Value result;
    while (begin != end) {
      auto ustr = begin->path().filename().u8string();
      std::string str;
      str.resize(ustr.size());
      memcpy(str.data(), ustr.data(), str.size());
      result["children"].append(file_info_to_json(read_file_info(*begin)));
      ++begin;
    }
    response = HttpResponse::newHttpJsonResponse(result);
  } else {
    Json::Value result;
    result["size"] = std::filesystem::file_size(path);

    response = HttpResponse::newHttpJsonResponse(result);
  }

  LOG_TRACE << req->getPath() << '\n';
  LOG_INFO << req->getPath() << '\n';

  response->addHeader("Access-Control-Allow-Origin", "*");
  callback(response);
}
void api::v1::Files::getFetch(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) const {
  auto jsonPtr = req->jsonObject();
  auto reportFailure = [&] {
    auto resp = HttpResponse::newNotFoundResponse();
    resp->addHeader("Access-Control-Allow-Origin", "*");

    callback(resp);
  };
  std::string path = req->getParameter("path");
  bool as_view = req->getParameter("view") == "true";

  if (path.empty()) {
    if (!jsonPtr) {
      reportFailure();
      return;
    }
    path = (*jsonPtr)["path"].asString();
  }
  auto host = req->getHeader("Host");
  std::cout << "fetch path: " << path << " from "<< host << "\n";
  if (!std::filesystem::exists(path)) {
    reportFailure();
    return;
  }
  std::optional<std::string> etag = generate_weak_etag(path);
  auto request_etag = req->getHeader("If-None-Match");
  if (!request_etag.empty() && etag && (*etag == request_etag)) {
    auto response = HttpResponse::newHttpResponse();
    response->setStatusCode(HttpStatusCode::k304NotModified);
    allow_cors(response);
    callback(response);
    return;
  }

  HttpResponsePtr response;
  if (std::filesystem::is_directory(path)) {
    std::filesystem::directory_iterator begin{path}, end;
    Json::Value result;
    while (begin != end) {
      auto ustr = begin->path().filename().u8string();

      ++begin;
    }
    // response = HttpResponse::newStreamResponse();
  } else {
    Json::Value result;
    response = HttpResponse::newFileResponse(path);
  }

  LOG_TRACE << req->getPath() << '\n';
  LOG_INFO << req->getPath() << '\n';

  // std::string fetch_type = "attachment";
  std::string fetch_type = as_view ? "inline" : "attachment";
  response->addHeader("Access-Control-Allow-Origin", "*");
  response->addHeader("Content-Disposition",
                      fetch_type + "; filename=\"" +
                          std::filesystem::path(path).filename().string() +
                          "\"");
  if (etag) {
    response->addHeader("ETag", *etag);
  }
  // if(response->getHeader("Content-Type").empty()){
  //   response->removeHeader("Content-Type");
  //   response->addHeader("Content-Type","text/html; charset=utf-8");
  // }
  callback(response);
}

#include <drogon/HttpResponse.h>
#include <drogon/drogon.h>
#include <drogon/utils/Utilities.h>
#include <sstream>
#include <string>
using namespace drogon;
int main() {
  app().registerHandlerViaRegex(
      "/", [](const HttpRequestPtr &req,
              std::function<void(const HttpResponsePtr &)> &&callback) {
        for (auto header : req->headers()) {
          std::cout << header.first << ": " << header.second << "\n";
        }
        auto auth = req->getHeader("authorization");
        if (!auth.empty()) {
          std::cout << auth << "\n";
          std::string type;
          std::stringstream ss{auth};
          std::string token;
          ss >> type >> std::ws;
          ss >> token;

          auto auth_content = utils::base64Decode(token);
          std::cout << type << ": " << auth_content << "\n";
        }
        std::cout << req->getBody() << "\n";

        auto resp = HttpResponse::newHttpResponse();
        resp->setBody("Good Job\n");
        callback(resp);
      });

  app()
      .setLogPath("./")
      .setLogLevel(trantor::Logger::kTrace)
      .addListener("0.0.0.0", 9812)
      .setThreadNum(0)
      .setClientMaxBodySize(100'000'000)
      .run();
}
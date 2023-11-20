#include <drogon/HttpResponse.h>
#include <drogon/drogon.h>
using namespace drogon;
int main() {
  app().registerHandlerViaRegex(
      "/", [](const HttpRequestPtr &req,
              std::function<void(const HttpResponsePtr &)> &&callback) {
        for (auto header : req->headers()) {
          std::cout << header.first << ": " << header.second << "\n";
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
#include <cmath>
#include <drogon/drogon.h>
using namespace drogon;
int main()
{
    app().setLogPath("./")
         .setLogLevel(trantor::Logger::kTrace)
         .addListener("0.0.0.0", 9812)
         .setThreadNum(0)
         .setClientMaxBodySize(std::pow(1024,3))
         .run();
}
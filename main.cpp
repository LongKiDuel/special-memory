#include "load_model.h"
#include "report_model.h"
#include <spdlog/spdlog.h>
int main(int argc, char **argv) {
  std::string path = argc > 1 ? argv[1] : "/home/nanami/Downloads/lucy.obj";
  auto model = load_model(path).value();

  SPDLOG_INFO(report_model_all(model));
}

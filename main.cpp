#include "load_model.h"
#include "report_model.h"
#include <spdlog/spdlog.h>
int main() {
  auto model = load_model("/home/nanami/Downloads/lucy.obj");

  SPDLOG_INFO(report_model_all(model));
}

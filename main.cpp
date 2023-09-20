#include "bin_model_exporter.h"
#include "load_model.h"
#include "mesh_pnu_format.h"
#include "report_model.h"
#include <spdlog/spdlog.h>
#include <string>
int main(int argc, char **argv) {
  std::string path = argc > 1 ? argv[1] : "/home/nanami/Downloads/lucy.obj";
  auto model = load_model(path).value();

  SPDLOG_INFO(report_model_all(model));
  int i{};
  for (auto &mesh : model.meshes) {
    Mesh_pnu_format m{mesh};
    m.rebuild_indices();
    SPDLOG_INFO("{}", m.report());
    if (argc > 2) {
      std::string prefix = i > 0 ? std::to_string(i) : "";
      bin_model_exporter(m, prefix + argv[2]);
    }
    i++;
  }
}

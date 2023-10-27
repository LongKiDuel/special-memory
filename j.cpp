#include "json-keyfilter.h"

#include <array>
#include <iostream>
#include <iterator>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <string_view>
#include <vector>
int main() {

  nlohmann::json j = {
      {"key1", "value1"}, {"key2", "value2"}, {"key3", "value3"}};

  auto filtered = filter_json_key(j, "key1", "key3");
  std::cout << filtered.dump(4) << std::endl;
  std::cout << filter_json_key_array(j, std::array{"key2", "key3"}).dump(4)
            << std::endl;
  std::cout << filter_json_drop_key_array(j, std::array{"key2", "key3"}).dump(4)
            << std::endl;

  std::string s{std::istreambuf_iterator<char>{std::cin}, {}};

  nlohmann::json input_json = nlohmann::json::parse(s);

  for (auto obj : input_json) {
    auto result =
        filter_json_key_array(obj, std::array{"Image", "Names", "Ports"});
    transform_json_value(result, "Names", [](auto names) { return names[0]; });
    transform_json_value(result, "Ports", [](auto ports) {
      for (auto &port : ports) {
        // std::cout << port << "\n";
        if (!port.contains("PublicPort")) {
          port =
              std::string("port: ") + std::to_string(int(port["PrivatePort"]));
          continue;
        }
        port = std::string(port["IP"]) + " " +
               std::to_string(int(port["PublicPort"])) + " -> " +
               std::to_string(int(port["PrivatePort"]));
      }
      return ports;
    });
    std::cout << result.dump(2) << "\n";
  }
}

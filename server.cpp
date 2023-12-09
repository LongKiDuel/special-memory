#include "local_ipc.h"
#include <csignal>
#include <iostream>

int main() {
  // In the server process
  static bool stop{};
  try {
    static auto server = local_ipc::create_server("/tmp/mysocket");
    signal(SIGINT, [](int) {
      stop = true;
      server->stop_new_connection();
    });
    while (true) {
      if (stop) {
        break;
      }
      std::string filename = server->receive_message();
      std::cout << "receive filename: " << filename << "\n";
    }
  } catch (const std::exception &e) {
    std::cerr << "Server error: " << e.what() << std::endl;
  }
}

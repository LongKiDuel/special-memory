#include "server.h"
int main() {
  // In the server process
  try {
    Server server("/tmp/mysocket");
    while (true) {
      server.acceptConnection();
      std::string filename = server.receiveMessage();
      server.openFile(filename);
    }
  } catch (const std::exception &e) {
    std::cerr << "Server error: " << e.what() << std::endl;
  }
}

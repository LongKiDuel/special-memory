#include "client.h"
#include <string>
int main(int argc, char **argv) {

  // In the client process
  try {
    Client client("/tmp/mysocket");
    std::string str;
    str = argv[1] ? argv[1] : "example.txt";
    client.sendMessage(str);
  } catch (const std::exception &e) {
    std::cerr << "Client error: " << e.what() << std::endl;
  }
}

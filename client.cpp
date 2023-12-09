#include "local_ipc.h"
#include <iostream>
#include <string>
int main(int argc, char **argv) {

  // In the client process
  try {
    auto client = local_ipc::create_client("/tmp/mysocket");
    std::string str;
    str = argv[1] ? argv[1] : "example.txt";
    client->send_message(str);
  } catch (const std::exception &e) {
    std::cerr << "Client error: " << e.what() << std::endl;
  }
}

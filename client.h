#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

class Client {
private:
  int sock;
  struct sockaddr_un server_addr;

public:
  Client(const std::string &socket_path) {
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
      throw std::runtime_error("Socket creation failed");
    }

    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path.c_str(),
            sizeof(server_addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
        0) {
      throw std::runtime_error("Connection failed");
    }
  }

  void sendMessage(const std::string &message) {
    send(sock, message.c_str(), message.length(), 0);
  }

  ~Client() { close(sock); }
};

#include "local_ipc.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace {

class Client_impl {
private:
  int sock;
  struct sockaddr_un server_addr;

public:
  Client_impl(const std::string &socket_path) {
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

  ~Client_impl() { close(sock); }
};
} // namespace

namespace local_ipc {
class Unix_client : public Client {
public:
  Unix_client(std::string path) : client{std::move(path)} {}
  void send_message(std::string message) { client.sendMessage(message); }

private:
  Client_impl client;
};
std::unique_ptr<Client> create_client(std::string path) {
  return std::make_unique<Unix_client>(std::move(path));
}
} // namespace local_ipc

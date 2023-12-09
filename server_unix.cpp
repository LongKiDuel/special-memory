#include "local_ipc.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace {

class Server_impl {
private:
  std::string path_;
  struct sockaddr_un address;
  int server_fd{}, client_fd{};

public:
  Server_impl(const std::string &socket_path) {
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
      throw std::runtime_error("Socket creation failed");
    }

    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, socket_path.c_str(),
            sizeof(address.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
      throw std::runtime_error("Bind failed");
    }

    if (listen(server_fd, 3) < 0) {
      throw std::runtime_error("Listen failed");
    }
    path_ = socket_path;
  }

  void acceptConnection() {
    client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd < 0) {
      throw std::runtime_error("Accept failed");
    }
  }

  std::string receiveMessage() {
    char buffer[1024] = {0};
    read(client_fd, buffer, 1024);
    return std::string(buffer);
  }

  void openFile(const std::string &filename) {
    std::fstream file;
    file.open(filename, std::ios::in);
    if (file.is_open()) {
      std::cout << "File opened successfully: " << filename << std::endl;
      // Perform file operations
      file.close();
    } else {
      std::cout << "Failed to open file: " << filename << std::endl;
    }
  }
  void stop_new_connection() { close(server_fd); }

  ~Server_impl() {
    close(client_fd);
    close(server_fd);
    unlink(address.sun_path);
  }
};
} // namespace

namespace local_ipc {
class Unix_server : public Server {
public:
  Unix_server(std::string path) : sv{std::move(path)} {}
  std::string receive_message() override {
    sv.acceptConnection();
    return sv.receiveMessage();
  }
  void stop_new_connection() override { sv.stop_new_connection(); }

private:
  Server_impl sv;
};
std::unique_ptr<Server> create_server(std::string path) {
  return std::make_unique<Unix_server>(std::move(path));
}
} // namespace local_ipc

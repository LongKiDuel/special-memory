#include <fstream>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

class Server {
private:
  int server_fd, client_fd;
  struct sockaddr_un address;

public:
  Server(const std::string &socket_path) {
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

  ~Server() {
    close(client_fd);
    close(server_fd);
    unlink(address.sun_path);
  }
};

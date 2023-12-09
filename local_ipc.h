#pragma once
#include <memory>
#include <string>

// those function will throw exception on failed.
namespace local_ipc {
class Server {
public:
  virtual ~Server() = default;
  virtual std::string receive_message() = 0;
  virtual void stop_new_connection() = 0;
};
class Client {
  virtual ~Client() = default;
  virtual void send_message(std::string message) = 0;
};

std::unique_ptr<Server> create_server(std::string path);
std::unique_ptr<Client> create_Client(std::string path);
} // namespace local_ipc
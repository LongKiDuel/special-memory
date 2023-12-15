#include <algorithm>
#include <cstdint>
#include <fmt/core.h>
#include <fmt/format.h>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <vector>
class Actor;
struct Message {
  uint64_t src_id_;
  uint64_t dest_id_;
  std::string body_;
};
class Actor_system {
public:
  Actor_system() {}
  void add_message(Message newMessage);

  std::queue<Message> message_queue_;
  std::vector<std::shared_ptr<Actor>> actors_;

  static uint64_t create_id() {
    static uint64_t id{};
    return id++;
  }
};
Actor_system static_system;

class Actor_factory {
public:
  template <typename T = Actor> static std::shared_ptr<T> create() {
    auto actor = std::make_shared<T>();
    static_system.actors_.push_back(actor);
    return actor;
  }
};

class Actor {

public:
  Actor() {}

  uint64_t get_id() const { return id_; }
  void send(uint64_t id, std::string body) {
    auto message = send_impl(id, body);
    static_system.add_message(message);
  }
  void receive(Message message) { receive_impl(message); }

protected:
  virtual Message send_impl(uint64_t id, std::string body) {
    return Message{get_id(), id, std::move(body)};
  }
  virtual void receive_impl(Message message) {
    std::cout << fmt::format("{} -> {}: {}\n", message.src_id_,
                             message.dest_id_, message.body_);
    auto next_id = get_id() + 1;
    next_id %= 10;
    send(next_id, message.body_);
  }

private:
  uint64_t id_{Actor_system::create_id()};
};

void Actor_system::add_message(Message newMessage) {

  message_queue_.push(newMessage);
  if (message_queue_.size() != 1) {
    // other task is ruuning let it do task.
    return;
  }
  while (!message_queue_.empty()) {
    auto top = std::move(message_queue_.front());
    actors_[top.dest_id_]->receive(top);
    message_queue_.pop(); // not remove the message immediately.
  }
}

int main() {
  std::vector<std::shared_ptr<Actor>> actors;
  for (int i = 0; i < 10; i++) {
    actors.push_back(Actor_factory::create());
  }

  actors[0]->send(4, "Hello");
}
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
class Actor_system;

struct Message {
  uint64_t src_id_;
  uint64_t dest_id_;
  std::string body_;
};
class Actor_system {
public:
  Actor_system() {}
  void add_message(Message newMessage);

  uint64_t create_id() { return id_base_++; }
  template <typename T = Actor> std::shared_ptr<T> create_actor() {
    auto actor = std::make_shared<T>(*this);
    actors_.push_back(actor);
    return actor;
  }

private:
  std::vector<std::shared_ptr<Actor>> actors_;
  std::queue<Message> message_queue_;
  uint64_t id_base_{};
};

class Actor {
public:
  Actor(Actor_system &system) : system_(&system), id_{system.create_id()} {}

  uint64_t get_id() const { return id_; }
  void send(uint64_t id, std::string body) {
    auto message = send_impl(id, body);
    system_->add_message(message);
  }
  void receive(Message message) { receive_impl(message); }

protected:
  virtual Message send_impl(uint64_t id, std::string body) {
    return Message{get_id(), id, std::move(body)};
  }
  virtual void receive_impl(Message message) {
    auto number = std::stoi(message.body_);
    std::cout << fmt::format("{} -> {}: {}\n", message.src_id_,
                             message.dest_id_, number);
    if (number >= 1000) {
      return;
    }
    auto next_id = get_id() + 1;
    next_id %= 10;
    send(next_id, std::to_string(number + 1));
  }

private:
  Actor_system *system_;
  uint64_t id_{};
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

  Actor_system system;
  std::vector<std::shared_ptr<Actor>> actors;
  for (int i = 0; i < 10; i++) {
    actors.push_back(system.create_actor());
  }

  actors[0]->send(4, "0");
}
// Copyright 2007 - 2021, Alan Antonuk and the rabbitmq-c contributors.
// SPDX-License-Identifier: mit

#include <cstdint>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <amqp.h>
#include <amqp_tcp_socket.h>

#include <assert.h>
#include <string>
#include <vector>

#include "utils.h"

#define SUMMARY_EVERY_US 1000000

static void run(amqp_connection_state_t conn) {
  uint64_t start_time = now_microseconds();
  int received = 0;
  int previous_received = 0;
  uint64_t previous_report_time = start_time;
  uint64_t next_summary_time = start_time + SUMMARY_EVERY_US;

  amqp_frame_t frame;

  uint64_t now;

  for (;;) {
    amqp_rpc_reply_t ret;
    amqp_envelope_t envelope;

    now = now_microseconds();
    if (now > next_summary_time) {
      int countOverInterval = received - previous_received;
      double intervalRate =
          countOverInterval / ((now - previous_report_time) / 1000000.0);
      printf("%d ms: Received %d - %d since last report (%d Hz)\n",
             (int)(now - start_time) / 1000, received, countOverInterval,
             (int)intervalRate);

      previous_received = received;
      previous_report_time = now;
      next_summary_time += SUMMARY_EVERY_US;
    }

    amqp_maybe_release_buffers(conn);
    ret = amqp_consume_message(conn, &envelope, NULL, 0);

    amqp_basic_ack(conn, envelope.channel, envelope.delivery_tag, 0);
    if (AMQP_RESPONSE_NORMAL != ret.reply_type) {
      if (AMQP_RESPONSE_LIBRARY_EXCEPTION == ret.reply_type &&
          AMQP_STATUS_UNEXPECTED_STATE == ret.library_error) {
        if (AMQP_STATUS_OK != amqp_simple_wait_frame(conn, &frame)) {
          return;
        }

        if (AMQP_FRAME_METHOD == frame.frame_type) {
          switch (frame.payload.method.id) {
          case AMQP_BASIC_ACK_METHOD:
            /* if we've turned publisher confirms on, and we've published a
             * message here is a message being confirmed.
             */
            break;
          case AMQP_BASIC_RETURN_METHOD:
            /* if a published message couldn't be routed and the mandatory
             * flag was set this is what would be returned. The message then
             * needs to be read.
             */
            {
              amqp_message_t message;
              ret = amqp_read_message(conn, frame.channel, &message, 0);
              if (AMQP_RESPONSE_NORMAL != ret.reply_type) {
                return;
              }

              amqp_destroy_message(&message);
            }

            break;

          case AMQP_CHANNEL_CLOSE_METHOD:
            /* a channel.close method happens when a channel exception occurs,
             * this can happen by publishing to an exchange that doesn't exist
             * for example.
             *
             * In this case you would need to open another channel redeclare
             * any queues that were declared auto-delete, and restart any
             * consumers that were attached to the previous channel.
             */
            return;

          case AMQP_CONNECTION_CLOSE_METHOD:
            /* a connection.close method happens when a connection exception
             * occurs, this can happen by trying to use a channel that isn't
             * open for example.
             *
             * In this case the whole connection must be restarted.
             */
            return;

          default:
            fprintf(stderr, "An unexpected method was received %u\n",
                    frame.payload.method.id);
            return;
          }
        }
      }

    } else {
      amqp_destroy_envelope(&envelope);
    }

    received++;
  }
}
namespace amqpp {
struct Connection_info {
  std::string hostname_;
  uint16_t port_;
};
} // namespace amqpp
namespace amqpp::raii {
struct Connection {
  Connection() : connection_(amqp_new_connection()) {}
  ~Connection() {}
  amqp_connection_state_t connection_{};
};
struct Socket {
  amqp_socket_t *socket_{};

  Socket(Connection &connection) {
    socket_ = amqp_tcp_socket_new(connection.connection_);
    assert(socket_);
    if (!socket_) {
      die("creating TCP socket");
    }
  }
  ~Socket() {}

  void connection_to(const Connection_info &destination) {
    auto error = amqp_socket_open(socket_, destination.hostname_.c_str(),
                                  destination.port_);
    assert(!error);
    if (error) {
      die("opening TCP socket");
    }
  }
};
} // namespace amqpp::raii
namespace amqpp {
struct Login_info {
  std::string virtual_host{};
  int max_channel_{}; // 0 for no limit.
  int max_frame_size_{AMQP_DEFAULT_FRAME_SIZE};
  int heartbeat_every_x_secones_{}; // 0 for no heartbeat
  amqp_sasl_method_enum auth_method{AMQP_SASL_METHOD_PLAIN};
  std::string user_name_;
  std::string password_;
};
Login_info get_default_login_info() {
  Login_info info;
  info.virtual_host = "/";
  info.user_name_ = "guest";
  info.password_ = "guest";
  return info;
}
class Connection;
class Channel {
public:
  int get_channel_id() { return channel_id_; }

private:
  Channel(int channel_id) : channel_id_(channel_id) {}
  friend class Connection;
  int channel_id_{};
};
class Bytes : std::vector<char> {
  using Base = std::vector<char>;

public:
  Bytes() = default;
  Bytes(const std::string &str) : Base{str.begin(), str.end()} {}
  Bytes(const char *str) : Bytes{std::string(str)} {}
  Bytes(std::vector<char> buffer) : Base{std::move(buffer)} {}
  Bytes(amqp_bytes_t buffer)
      : Base{reinterpret_cast<const char *>(buffer.bytes),
             reinterpret_cast<const char *>(buffer.bytes) + buffer.len} {}

  operator amqp_bytes_t() {
    amqp_bytes_t bytes;
    bytes.bytes = data();
    bytes.len = size();
    return bytes;
  }
  operator amqp_bytes_t() const {
    amqp_bytes_t bytes;
    bytes.bytes = const_cast<char *>(data());
    bytes.len = size();
    return bytes;
  }
};
class Argument_table {
public:
  amqp_table_t get_table() const { return amqp_empty_table; }
  std::vector<amqp_table_entry_t> entries_;
};
class Queue {
public:
  Queue(Bytes name) { queue_name_ = name; }
  Queue() {}
  const Bytes &get_name() const { return queue_name_; }

private:
  Bytes queue_name_{};
};
struct Queue_declare_info {
  Bytes queue_name; // keep it empty will generate a queue with random name?
  bool passive_{};
  bool durable_{};
  bool exclusive_{};
  bool auto_delete_{};
  Argument_table arguments_{};
};
struct Queue_bind_info {
  Bytes queue_name_{};
  Bytes exchange_name_{};
  Bytes bindingkey_name_{};
  Argument_table arguments_{};
};
struct Queue_consume_info {
  Bytes queue_name_;
  Bytes cosumer_tag_;
  bool no_local_;
  bool no_manually_ack_{};
  bool exclusive_{};
  Argument_table arguments_{};
};
class Connection {
public:
  Connection(const Connection_info host_info) : socket_(connection_) {
    socket_.connection_to(host_info);
  }

  auto get_connection_handle() { return connection_.connection_; }

  auto login(const Login_info &info) {
    auto result = amqp_login(get_connection_handle(), info.virtual_host.c_str(),
                             info.max_channel_, info.max_frame_size_,
                             info.heartbeat_every_x_secones_, info.auth_method,
                             info.user_name_.c_str(), info.password_.c_str());
    die_on_amqp_error(result, "Logging in");
  }

  Channel create_channel() {
    auto cid = new_channel_id_generator++;
    amqp_channel_open(get_connection_handle(), cid);
    die_on_amqp_error(amqp_get_rpc_reply(get_connection_handle()),
                      "Opening channel");
    return Channel(cid);
  }

  Queue declare_queue(Channel &channel, const Queue_declare_info &info) {
    amqp_queue_declare_ok_t *r = amqp_queue_declare(
        get_connection_handle(), channel.get_channel_id(), info.queue_name,
        info.passive_, info.durable_, info.exclusive_, info.auto_delete_,
        info.arguments_.get_table());
    die_on_amqp_error(amqp_get_rpc_reply(get_connection_handle()),
                      "Declaring queue");
    Bytes queue_name = r->queue;
    return Queue{queue_name};
  }
  void bind_queue(Channel &channel, const Queue_bind_info &info) {
    amqp_queue_bind(get_connection_handle(), channel.get_channel_id(),
                    info.queue_name_, info.exchange_name_,
                    info.bindingkey_name_, info.arguments_.get_table());
    die_on_amqp_error(amqp_get_rpc_reply(get_connection_handle()),
                      "Binding queue");
  }

  void consume_queue(Channel &channel, const Queue_consume_info &info) {
    amqp_basic_consume(get_connection_handle(), channel.get_channel_id(),
                       info.queue_name_, info.cosumer_tag_, info.no_local_,
                       info.no_manually_ack_, info.exclusive_,
                       info.arguments_.get_table());
    die_on_amqp_error(amqp_get_rpc_reply(get_connection_handle()), "Consuming");
  }

private:
  raii::Connection connection_{};
  raii::Socket socket_;
  int new_channel_id_generator{1}; // create channel start from 1.
};
} // namespace amqpp

int main(int argc, char const *const *argv) {

  char const *exchange;
  char const *bindingkey;

  if (argc < 3) {
    fprintf(stderr, "Usage: amqp_consumer host port\n");
    return 1;
  }
  amqpp::Connection_info connection_info{};
  {
    connection_info.hostname_ = argv[1];
    connection_info.port_ = atoi(argv[2]);
  }
  exchange = "amq.direct";   /* argv[3]; */
  bindingkey = "test queue"; /* argv[4]; */
  amqpp::Connection connection{connection_info};

  amqp_connection_state_t conn = connection.get_connection_handle();
  connection.login(amqpp::get_default_login_info());

  auto channel = connection.create_channel();
  amqpp::Queue queue;
  {
    // from amqp_empty_bytes to amqp_cstring_bytes("task_queue")
    // use named queue to make mesagge only send to one client in same queue.
    //
    // turn off queue auto delete.
    amqpp::Queue_declare_info info{};
    info.durable_ = true;
    info.queue_name = "task_queue";
    queue = connection.declare_queue(channel, info);
  }

  amqpp::Queue_bind_info bind_info{};
  bind_info.queue_name_ = queue.get_name();
  bind_info.exchange_name_ = exchange;
  bind_info.bindingkey_name_ = bindingkey;
  connection.bind_queue(channel, bind_info);

  amqpp::Queue_consume_info consume_info{};
  consume_info.queue_name_ = queue.get_name();

  connection.consume_queue(channel, consume_info);

  run(conn);

  die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS),
                    "Closing channel");
  die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS),
                    "Closing connection");
  die_on_error(amqp_destroy_connection(conn), "Ending connection");

  return 0;
}
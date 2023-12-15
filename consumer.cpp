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

private:
  raii::Connection connection_{};
  raii::Socket socket_;
};
} // namespace amqpp

int main(int argc, char const *const *argv) {

  char const *exchange;
  char const *bindingkey;

  amqp_bytes_t queuename;

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

  amqp_channel_open(conn, 1);
  die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");

  {
    // from amqp_empty_bytes to amqp_cstring_bytes("task_queue")
    // use named queue to make mesagge only send to one client in same queue.
    //
    // turn off queue auto delete.
    amqp_queue_declare_ok_t *r =
        amqp_queue_declare(conn, 1, amqp_cstring_bytes("task_queue"), 0, 1, 0,
                           0, amqp_empty_table);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring queue");
    queuename = amqp_bytes_malloc_dup(r->queue);
    if (queuename.bytes == NULL) {
      fprintf(stderr, "Out of memory while copying queue name");
      return 1;
    }
  }

  amqp_queue_bind(conn, 1, queuename, amqp_cstring_bytes(exchange),
                  amqp_cstring_bytes(bindingkey), amqp_empty_table);
  die_on_amqp_error(amqp_get_rpc_reply(conn), "Binding queue");

  amqp_basic_consume(conn, 1, queuename, amqp_empty_bytes, 0, 0, 0,
                     amqp_empty_table);
  die_on_amqp_error(amqp_get_rpc_reply(conn), "Consuming");

  run(conn);

  amqp_bytes_free(queuename);

  die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS),
                    "Closing channel");
  die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS),
                    "Closing connection");
  die_on_error(amqp_destroy_connection(conn), "Ending connection");

  return 0;
}
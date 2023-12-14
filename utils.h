#pragma once

#include <amqp.h>
#include <chrono>
#include <cstdint>
#include <iostream>
uint64_t now_microseconds() {
  return std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::high_resolution_clock::now().time_since_epoch())
      .count();
}

auto die_on_error(auto x, auto context) {
  if (x < 0) {
    fprintf(stderr, "%s: %s\n", context, amqp_error_string2(x));
    exit(1);
  }
}
auto die_on_amqp_error(amqp_rpc_reply_t_ x, auto context) {
  switch (x.reply_type) {
  case AMQP_RESPONSE_NORMAL:
    return;

  case AMQP_RESPONSE_NONE:
    fprintf(stderr, "%s: missing RPC reply type!\n", context);
    break;

  case AMQP_RESPONSE_LIBRARY_EXCEPTION:
    fprintf(stderr, "%s: %s\n", context, amqp_error_string2(x.library_error));
    break;

  case AMQP_RESPONSE_SERVER_EXCEPTION:
    switch (x.reply.id) {
    case AMQP_CONNECTION_CLOSE_METHOD: {
      amqp_connection_close_t *m = (amqp_connection_close_t *)x.reply.decoded;
      fprintf(stderr, "%s: server connection error %uh, message: %.*s\n",
              context, m->reply_code, (int)m->reply_text.len,
              (char *)m->reply_text.bytes);
      break;
    }
    case AMQP_CHANNEL_CLOSE_METHOD: {
      amqp_channel_close_t *m = (amqp_channel_close_t *)x.reply.decoded;
      fprintf(stderr, "%s: server channel error %uh, message: %.*s\n", context,
              m->reply_code, (int)m->reply_text.len,
              (char *)m->reply_text.bytes);
      break;
    }
    default:
      fprintf(stderr, "%s: unknown server error, method id 0x%08X\n", context,
              x.reply.id);
      break;
    }
    break;
  }

  exit(1);
}
void die(auto text) {
  std::cerr << "Die: " << text << std::endl;
  exit(1);
}
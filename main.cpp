#include <asio.hpp>
#include <asio/registered_buffer.hpp>
#include <iostream>
#include <memory>
#include <string>

void send_broadcast(asio::io_context &io_context) {
  static asio::ip::udp::socket socket(
      io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0));
  socket.set_option(asio::socket_base::broadcast(true));

  const std::string message = "Network discovery probe";
  static asio::ip::udp::endpoint receiver_endpoint(
      asio::ip::address_v4::broadcast(), 8888);

  // Prepare a buffer for the incoming message. Use a shared_ptr to manage the
  // lifetime.
  static auto receive_buffer = std::make_shared<std::vector<char>>(1024);

  socket.async_send_to(
      asio::buffer(message), receiver_endpoint,
      [&](std::error_code /*ec*/, std::size_t /*bytes_sent*/) {
        // Start the receive operation after the send completes
        socket.async_receive_from(
            asio::buffer(*receive_buffer), receiver_endpoint,
            [&io_context](const std::error_code &ec, std::size_t bytes_recvd) {
              if (!ec && bytes_recvd > 0) {
                std::cout << "Received response from: " << receiver_endpoint
                          << "\n";
                std::cout << "Message: "
                          << std::string(receive_buffer->begin(),
                                         receive_buffer->begin() + bytes_recvd)
                          << "\n";
              }
            });
      });
}

// void listen_for_responses(asio::io_context& io_context) {
//     asio::ip::udp::socket socket(io_context,
//     asio::ip::udp::endpoint(asio::ip::udp::v4(), 8888)); std::array<char,
//     1024> recv_buffer; asio::ip::udp::endpoint sender_endpoint;

//     socket.async_receive_from(
//         asio::buffer(recv_buffer), sender_endpoint,
//         [&](std::error_code ec, std::size_t bytes_recvd) {
//             if (!ec && bytes_recvd > 0) {
//                 std::cout << "Received response from: " << sender_endpoint <<
//                 "\n"; std::cout << "Message: " <<
//                 std::string(recv_buffer.data(), bytes_recvd) << "\n";
//             }
//             // Could set up another async receive here for continuous
//             listening
//         });
// }
void listen_for_responses(asio::io_context &io_context) {
  static asio::ip::udp::socket socket(
      io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), 8888));
  static std::array<char, 1024> recv_buffer;
  static asio::ip::udp::endpoint sender_endpoint;

  static std::function<void(const std::error_code &, std::size_t)>
      receive_handler;

  receive_handler = [&](const std::error_code &ec, std::size_t bytes_recvd) {
    if (!ec && bytes_recvd > 0) {
      std::cout << "Received response from: " << sender_endpoint << "\n";
      std::cout << "Message: " << std::string(recv_buffer.data(), bytes_recvd)
                << "\n";
    }

    // Continue receiving
    socket.async_receive_from(asio::buffer(recv_buffer), sender_endpoint,
                              receive_handler);
    auto static buf = asio::buffer("hello world");
    socket.async_send_to(buf, sender_endpoint,
                         [](const std::error_code &err, std::size_t) {
                           std::cout << "send back:" << err.message() << "\n";
                         });
  };

  // Start receiving
  socket.async_receive_from(asio::buffer(recv_buffer), sender_endpoint,
                            receive_handler);
}

int main(int argc, char **argv) {
  asio::io_context io_context;
  // Set a signal handler to perform a clean shutdown
  asio::signal_set signals(io_context, SIGINT, SIGTERM);
  signals.async_wait([&](const std::error_code &, int) { io_context.stop(); });
  if (argc > 1) {
    // Send a broadcast message
    send_broadcast(io_context);
  } else {
    // Listen for any responses
    listen_for_responses(io_context);
  }

  // Run the IO context to perform asynchronous operations
  io_context.run();

  return 0;
}

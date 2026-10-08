#include <cstdlib>
#include <iostream>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>

#include "Server.h"

constexpr short TCP_PORT = 9000;
constexpr short UDP_PORT = 9001;

int main(int argc, char* argv[]) {
    const auto tcp_port = argc > 1
        ? static_cast<unsigned short>(std::atoi(argv[1]))
        : static_cast<unsigned short>(TCP_PORT);
    const auto udp_port = argc > 2
        ? static_cast<unsigned short>(std::atoi(argv[2]))
        : static_cast<unsigned short>(UDP_PORT);

    try {
        boost::asio::io_context io_context;
        Server server(io_context, tcp_port, udp_port);

        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](const boost::system::error_code&, int) {
            server.stop();
            io_context.stop();
        });

        io_context.run();
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
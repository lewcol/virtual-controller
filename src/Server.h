#pragma once

#include <boost/asio/io_context.hpp>

#include "SessionManager.h"
#include "TCPServer.h"
#include "UDPServer.h"

class Server {
public:
    Server(boost::asio::io_context& io_context,
           unsigned short tcp_port,
           unsigned short udp_port);
    void stop();

private:
    SessionManager session_manager_;
    TCPServer tcp_server_;
    UDPServer udp_server_;
};
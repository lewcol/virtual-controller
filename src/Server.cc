#include "Server.h"

Server::Server(boost::asio::io_context& io_context,
			   unsigned short tcp_port,
			   unsigned short udp_port)
	: session_manager_(udp_port),
	  tcp_server_(io_context, tcp_port, session_manager_),
	  udp_server_(io_context, udp_port, session_manager_) {}

void Server::stop()
{
    tcp_server_.stop();
    udp_server_.stop();
    session_manager_.shutdown();
}
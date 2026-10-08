#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

class SessionManager;

class TCPServer {
public:
	TCPServer(boost::asio::io_context& io_context,
			  unsigned short port,
			  SessionManager& session_manager);
	void stop();

private:
	void accept();
	void handle_accept(const boost::system::error_code& error,
					   boost::asio::ip::tcp::socket socket);

	boost::asio::ip::tcp::acceptor acceptor_;
	SessionManager& session_manager_;
};


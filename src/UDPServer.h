#pragma once

#include <array>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>

class SessionManager;

class UDPServer {
public:
	UDPServer(boost::asio::io_context& io_context,
	          unsigned short port,
	          SessionManager& session_manager);
	void stop();

private:
	void receive();
	void handle_receive(const boost::asio::ip::udp::endpoint& endpoint,
						const boost::system::error_code& error,
						std::size_t bytes_transferred);

	boost::asio::ip::udp::socket socket_;
	boost::asio::ip::udp::endpoint remote_endpoint_;
	std::array<char, 1024> receive_buffer_{};
	SessionManager& session_manager_;
};


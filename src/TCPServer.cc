#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "SessionManager.h"
#include "TCPServer.h"

using boost::asio::ip::tcp;

TCPServer::TCPServer(boost::asio::io_context& io_context,
                     unsigned short port,
                     SessionManager& session_manager)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
      session_manager_(session_manager) {
    accept();
}

void TCPServer::stop()
{
    boost::system::error_code error;
    acceptor_.close(error);
}

void TCPServer::accept() {
    acceptor_.async_accept(
        [this](const boost::system::error_code& error, tcp::socket socket) {
            handle_accept(error, std::move(socket));
        });
}

void TCPServer::handle_accept(const boost::system::error_code& error,
                              tcp::socket socket) {
    if (!error) {
        session_manager_.createSession(std::move(socket));
    }
    if (acceptor_.is_open()) {
        accept();
    }
}
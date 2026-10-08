#include "UDPServer.h"
#include "SessionManager.h"

UDPServer::UDPServer(boost::asio::io_context& io_context,
                     unsigned short port,
                     SessionManager& session_manager)
    : socket_(io_context, boost::asio::ip::udp::endpoint(
                boost::asio::ip::udp::v4(), port)),
      session_manager_(session_manager) {
    session_manager_.setUdpPort(socket_.local_endpoint().port());
    receive();
}

void UDPServer::stop()
{
    boost::system::error_code error;
    socket_.close(error);
}

void UDPServer::receive() {
    socket_.async_receive_from(
        boost::asio::buffer(receive_buffer_),
        remote_endpoint_,
        [this](const boost::system::error_code& error,
               std::size_t bytes_transferred) {
            handle_receive(remote_endpoint_, error, bytes_transferred);
        });
}

void UDPServer::handle_receive(const boost::asio::ip::udp::endpoint& endpoint,
                               const boost::system::error_code& error,
                               std::size_t bytes_transferred) {
    if (!error) {
        session_manager_.handleUdpPacket(
            endpoint,
            reinterpret_cast<const std::byte*>(receive_buffer_.data()),
            bytes_transferred);
    }

    if (socket_.is_open()) {
        receive();
    }
}
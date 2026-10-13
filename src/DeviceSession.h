#pragma once

#include <array>
#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <boost/uuid/uuid.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using boost::uuids::uuid;

class SessionManager;
class ClientUpdate;

class DeviceSession
    : public std::enable_shared_from_this<DeviceSession> {
public:
    DeviceSession(tcp::socket socket,
                  SessionManager& session_manager)
        : socket_(std::move(socket)),
          session_manager_(session_manager),
          negotiation_timer_(socket_.get_executor()) {}

    void startNegotiation();
    bool validateUpdate(const udp::endpoint& sender, const ClientUpdate& message);
    void sendSessionInfo(const uuid& session_id,
                         std::string session_token,
                         std::uint32_t controller_id,
                         std::uint16_t udp_port);
    bool acceptSequence(std::uint32_t sequence);
    bool acceptUdpEndpoint(const udp::endpoint& endpoint);
    void close();
    const std::optional<std::string>& getSessionToken() const { return session_token_; }
    const std::optional<std::uint32_t>& getControllerId() const { return controller_id_; }

private:
    void readHeader();
    void readBody();
    void handleMessage();
    void handleHeartbeat();
    void send(std::string message);
    void send_next();
    void createNegotiationTimeout();
    
    inline void cancelNegotiationTimeout() 
    { 
        negotiated_ = true;
        negotiation_timer_.cancel(); 
    }

    tcp::socket socket_;
    std::uint16_t udp_port_ = 0;
    SessionManager& session_manager_;
    std::array<std::uint8_t, 4> header_{};
    std::vector<std::uint8_t> body_;
    boost::asio::steady_timer negotiation_timer_;
    std::optional <uuid> session_id_;
    std::optional<std::string> session_token_;
    std::optional<std::uint32_t> controller_id_;
    std::optional<std::uint32_t> last_sequence_;
    std::optional<udp::endpoint> udp_endpoint_;
    std::deque<std::string> pending_messages_;
    bool negotiated_ = false;
    bool sending_ = false;
    bool closed_ = false;
};
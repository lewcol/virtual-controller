#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <string>
#include <cstddef>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>

using boost::uuids::uuid;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class DeviceSession;
class ControllerManager;

class SessionManager {
public:
    SessionManager(unsigned short udp_port, ControllerManager& controller_manager)
        : udp_port_(udp_port),
          controller_manager_(controller_manager) {}
    void createSession(tcp::socket socket);
    void setUdpPort(unsigned short udp_port) { udp_port_ = udp_port; }
    void registerSession(std::shared_ptr<DeviceSession> session,
                         std::string device_name);
    void handleUdpPacket(const udp::endpoint& sender,
                         const std::byte* data,
                         std::size_t size);
    void shutdown();
    void negotiationTimeout(std::shared_ptr<DeviceSession>& session);

private:
    unsigned short udp_port_;
    ControllerManager& controller_manager_;
    std::vector<std::shared_ptr<DeviceSession>> pendingSessions_;
    std::unordered_map<uuid, std::shared_ptr<DeviceSession>> verifiedSessions_;
};
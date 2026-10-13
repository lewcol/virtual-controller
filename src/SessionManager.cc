#include <array>
#include <algorithm>
#include <stdexcept>
#include <openssl/rand.h>

#include "SessionManager.h"
#include "DeviceSession.h"
#include "ControllerManager.h"
#include "messages.pb.h"

// Begin DeviceSession negotiation after TCP connection is established
void SessionManager::createSession(tcp::socket socket) {
    auto session = std::make_shared<DeviceSession>(
        std::move(socket), *this);
    pendingSessions_.push_back(session);
    session->startNegotiation();
}

// Register a session after successful negotiation and send session info
void SessionManager::registerSession(
        std::shared_ptr<DeviceSession> session,
        std::string device_name) {
    const auto session_id = boost::uuids::random_generator()();

    std::array<unsigned char, 32> token_bytes{};
    if (RAND_bytes(token_bytes.data(), token_bytes.size()) != 1) {
        throw std::runtime_error("failed to generate session token");
    }

    const std::string session_token(
        reinterpret_cast<const char*>(token_bytes.data()),
        token_bytes.size());

    // Remove the session from pendingSessions_ and add it to verifiedSessions_
    auto pending = std::find(
        pendingSessions_.begin(), pendingSessions_.end(), session);
    if (pending != pendingSessions_.end()) {
        pendingSessions_.erase(pending);
    }

    // Obtain a controller ID for the session
    int controller_id = controller_manager_.getController();

    verifiedSessions_[session_id] = session;
    session->sendSessionInfo(
        session_id,
        session_token,
        controller_id,
        udp_port_);
}

// Handle incoming UDP messages
void SessionManager::handleUdpPacket(const udp::endpoint& sender,
                                     const std::byte* data,
                                     std::size_t size)
{
    ClientUpdate message;
    if (!message.ParseFromArray(data, static_cast<int>(size)) ||
        message.session_id().size() != 16 ||
        message.session_token().empty() ||
        !message.has_update()) {
        return;
    }

    uuid session_id;
    std::copy_n(
        message.session_id().begin(),
        session_id.size(),
        session_id.begin());

    const auto found = verifiedSessions_.find(session_id);
    if (found == verifiedSessions_.end()) {
        return;
    }

    const auto& session = found->second;
    if (!session->validateUpdate(sender, message)) {
        return;
    }

    // message validated
    int controller_id = *session->getControllerId();
    controller_manager_.updateController(controller_id, message.update());
}

void SessionManager::negotiationTimeout(std::shared_ptr<DeviceSession>& session)
{
    session->close();
    auto pending = std::find(
        pendingSessions_.begin(), pendingSessions_.end(), session);
    if (pending != pendingSessions_.end()) {
        pendingSessions_.erase(pending);
    }
}

void SessionManager::shutdown()
{
    for (const auto& session : pendingSessions_) {
        session->close();
    }

    for (const auto& [session_id, session] : verifiedSessions_) {
        session->close();
    }

    pendingSessions_.clear();
    verifiedSessions_.clear();
}

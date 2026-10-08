#include <array>
#include <algorithm>
#include <stdexcept>
#include <openssl/rand.h>

#include "DeviceSession.h"
#include "SessionManager.h"
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

    verifiedSessions_[session_id] = session;
    session->sendSessionInfo(
        session_id,
        session_token,
        next_controller_id_++,
        udp_port_);
}

// Validate and Authenticate incoming UDP packet, process update
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
    const auto& session_token = session->getSessionToken();
    if (!session_token || message.session_token() != *session_token) {
        return;
    }

    if (!session->acceptUdpEndpoint(sender)) {
        return;
    }

    const auto& update = message.update();
    if (!session->acceptSequence(update.sequence())) {
        return;
    }

    // to-do: implement virtual controller interface and process update
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

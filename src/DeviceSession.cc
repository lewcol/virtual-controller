#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <google/protobuf/message_lite.h>

#include "DeviceSession.h"
#include "SessionManager.h"
#include "messages.pb.h"

namespace {

constexpr std::uint32_t max_message_size = 64 * 1024;

// convert network-order bytes to host-order uint32_t
std::uint32_t decode_u32_be(const std::array<std::uint8_t, 4>& bytes)
{
    return (static_cast<std::uint32_t>(bytes[0]) << 24) |
           (static_cast<std::uint32_t>(bytes[1]) << 16) |
           (static_cast<std::uint32_t>(bytes[2]) << 8) |
           static_cast<std::uint32_t>(bytes[3]);
}

// convert host-order uint32_t to network-order bytes
std::array<std::uint8_t, 4> encode_u32_be(std::uint32_t value)
{
    return {
        static_cast<std::uint8_t>(value >> 24),
        static_cast<std::uint8_t>(value >> 16),
        static_cast<std::uint8_t>(value >> 8),
        static_cast<std::uint8_t>(value)
    };
}

} // namespace

void DeviceSession::startNegotiation()
{
    createNegotiationTimeout();
    readHeader();
}

// Timeout that deletes the pending session if negotiation has not succeeded within 5 seconds
void DeviceSession::createNegotiationTimeout()
{
    negotiation_timer_.expires_after(std::chrono::seconds(5));
    auto self = weak_from_this();
    negotiation_timer_.async_wait([self](const boost::system::error_code& error) {
        if (error == boost::asio::error::operation_aborted) {
            return;
        }

        if (auto session = self.lock()) {        
            session->session_manager_.negotiationTimeout(session);
        }
    });
}

// Async read the header of the next message and then read the body
void DeviceSession::readHeader()
{
    // shared_from_this to avoid dangling pointer in async callback
    auto self = shared_from_this();

    boost::asio::async_read(
        socket_,
        boost::asio::buffer(header_),
        [self](const boost::system::error_code& error, std::size_t) {
            if (error) {
                return;
            }

            const auto body_size = decode_u32_be(self->header_);
            if (body_size == 0 || body_size > max_message_size) {
                return;
            }

            self->body_.resize(body_size);
            self->readBody();
        });
}

// Async read the body of the next message and then handle it
void DeviceSession::readBody()
{
    auto self = shared_from_this();

    boost::asio::async_read(
        socket_,
        boost::asio::buffer(body_),
        [self](const boost::system::error_code& error, std::size_t) {
            if (error) {
                return;
            }

            self->handleMessage();
        });
}

// Handle the received message based on its type
void DeviceSession::handleMessage()
{
    ClientMessage message;
    if (!message.ParseFromArray(body_.data(), static_cast<int>(body_.size()))) {
        return;
    }

    switch (message.payload_case()) {
    // Handle the Hello message, register the session
    case ClientMessage::kHello:
        if (negotiated_) {
            return;
        }

        cancelNegotiationTimeout();
        session_manager_.registerSession(
            shared_from_this(),
            message.hello().device_name());
        readHeader();
        break;

    // Handle the Heartbeat message, update the last sequence number
    case ClientMessage::kHeartbeat:
        if (!negotiated_) {
            return;
        }

        handleHeartbeat();
        readHeader();
        break;

    case ClientMessage::PAYLOAD_NOT_SET:
        return;
    }
}

void DeviceSession::handleHeartbeat()
{
    // to-do: implement heartbeat
}

// Send the session info to the client after successful negotiation
void DeviceSession::sendSessionInfo(const uuid& session_id,
                                    std::string session_token,
                                    std::uint32_t controller_id,
                                    std::uint16_t udp_port)
{
    session_id_ = session_id;
    session_token_ = session_token;
    controller_id_ = controller_id;

    ServerMessage message;
    auto* info = message.mutable_session_info();
    info->set_session_id(
        reinterpret_cast<const char*>(session_id.begin()),
        session_id.size());
    info->set_session_token(std::move(session_token));
    info->set_controller_id(controller_id);
    info->set_udp_port(udp_port);

    // Serialize the message with a length prefix and send it asynchronously
    std::string serialized;
    if (!message.SerializeToString(&serialized)) {
        return;
    }

    auto self = shared_from_this();
    std::string response;

    // Prepend the message length as a big-endian unsigned 32-bit integer
    const auto frame_header = encode_u32_be(
        static_cast<std::uint32_t>(serialized.size()));
    response.append(
        reinterpret_cast<const char*>(frame_header.data()),
        frame_header.size());
    response.append(serialized);

    send(response);
}

void DeviceSession::send(std::string message)
{
    if (closed_) {
        return;
    }

    pending_messages_.push_back(std::move(message));

    if (!sending_) {
        send_next();
    }
}

void DeviceSession::send_next()
{
    if (pending_messages_.empty()) {
        sending_ = false;
        return;
    }

    sending_ = true;
    auto self = shared_from_this();
    const auto& message = pending_messages_.front();

    boost::asio::async_write(
        socket_,
        boost::asio::buffer(message),
        [self](const boost::system::error_code& error, std::size_t) {
            if (error) {
                self->sending_ = false;
                return;
            }

            self->pending_messages_.pop_front();
            self->send_next();
        });
}

// Check if the received sequence number is valid and update the last sequence number
bool DeviceSession::acceptSequence(std::uint32_t sequence)
{
    if (last_sequence_) {
    // A negative result will wrap around to a large positive number
        uint32_t seq_diff = sequence - *last_sequence_;
        // We reject diffs equal/larger than 2^31 as older
        if (seq_diff == 0 || seq_diff >= 0x80000000u) {
            return false;
        }
    }

    last_sequence_ = sequence;
    return true;
}

// Close the socket and cancel the negotiation timer
void DeviceSession::close()
{
    if (closed_) {
        return;
    }

    closed_ = true;
    boost::system::error_code ec;

    negotiation_timer_.cancel();
    socket_.shutdown(tcp::socket::shutdown_both, ec);
    socket_.close(ec);
}

bool DeviceSession::acceptUdpEndpoint(const udp::endpoint& endpoint)
{
    if (!udp_endpoint_) {
        udp_endpoint_.emplace(endpoint);
        return true;
    }

    return *udp_endpoint_ == endpoint;
}
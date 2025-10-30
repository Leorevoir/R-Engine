#include "R-Engine/Plugins/NetworkPlugin.hpp"
#include "R-Engine/Application.hpp"
#include "R-Engine/Core/Clock.hpp"
#include "R-Engine/Core/Logger.hpp"
#include "R-Engine/Plugins/Plugin.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

namespace r::net {

namespace {

/* --- Internal Helper Functions --- */

/**
 * @brief Serializes a Packet struct into a byte vector for transmission.
 * @param packet The Packet object to serialize.
 * @return A std::vector<u8> containing the serialized data.
 */
std::vector<u8> serializePacket(const Packet &packet)
{
    std::vector<u8> buffer;
    buffer.resize(24);
    size_t offset = 0;

    auto write16 = [&](u16 v) {
        v = htons(v);
        memcpy(&buffer[offset], &v, 2);
        offset += 2;
    };
    auto write32 = [&](u32 v) {
        v = htonl(v);
        memcpy(&buffer[offset], &v, 4);
        offset += 4;
    };
    auto write8 = [&](u8 v) { buffer[offset++] = v; };

    write16(packet.magic);
    write8(packet.version);
    write8(packet.flags);
    write32(packet.sequence);
    write32(packet.ackBase);
    write32(packet.ackBits);
    write8(packet.channel);
    write16(static_cast<u16>(packet.payload.size()));///< Use actual payload size
    write32(packet.clientId);
    write8(packet.command);

    /* Append the payload data to the header. */
    buffer.insert(buffer.end(), packet.payload.begin(), packet.payload.end());
    return buffer;
}

/**
 * @brief Deserializes a byte vector into a Packet struct.
 * @param buffer The byte vector to deserialize.
 * @return The deserialized Packet object.
 */
Packet deserializePacket(const std::vector<u8> &buffer)
{
    Packet packet = {};
    if (buffer.size() < 24) {
        return packet;
    }
    size_t offset = 0;

    auto read16 = [&]() -> u16 {
        uint16_t v;
        memcpy(&v, &buffer[offset], 2);
        offset += 2;
        return ntohs(v);
    };
    auto read32 = [&]() -> u32 {
        uint32_t v;
        memcpy(&v, &buffer[offset], 4);
        offset += 4;
        return ntohl(v);
    };
    auto read8 = [&]() -> u8 { return buffer[offset++]; };

    packet.magic = read16();
    packet.version = read8();
    packet.flags = read8();
    packet.sequence = read32();
    packet.ackBase = read32();
    packet.ackBits = read32();
    packet.channel = read8();
    packet.size = read16();///< This is the payload size
    packet.clientId = read32();
    packet.command = read8();

    /* Ensure the buffer is large enough for the declared payload size. */
    if (buffer.size() >= offset + packet.size) {
        /* Explicitly cast offsets to the iterator's difference_type (ptrdiff_t)
        to prevent sign-conversion warnings/errors. */
        auto start_it = buffer.begin() + static_cast<ptrdiff_t>(offset);
        auto end_it = start_it + static_cast<ptrdiff_t>(packet.size);
        packet.payload.assign(start_it, end_it);
    }
    return packet;
}

/* --- Reliability Helper Functions --- */

/**
 * @brief Updates the acknowledgment bitfield based on a newly received sequence number.
 * @param conn The connection state.
 * @param received_sequence The sequence number of the packet just received.
 */
void process_incoming_sequence(Connection &conn, u32 received_sequence)
{
    /* If this packet is old (outside our 32-packet window), ignore it. */
    if (received_sequence < conn.remote_sequence && conn.remote_sequence - received_sequence > 32) {
        return;
    }

    if (received_sequence > conn.remote_sequence) {
        u32 diff = received_sequence - conn.remote_sequence;
        if (diff < 32) {
            conn.ack_bits <<= diff;
        } else {
            conn.ack_bits = 0;
        }
        conn.remote_sequence = received_sequence;
    }

    u32 diff = conn.remote_sequence - received_sequence;
    if (diff < 32) {
        conn.ack_bits |= (1 << diff);
    }
}

/**
 * @brief Processes the acknowledgment data from an incoming packet.
 * @details Removes packets from our sent buffer that the remote peer has confirmed receiving.
 * @param conn The connection state.
 * @param ack_base The base sequence number from the remote peer's ack.
 * @param ack_bits The bitfield from the remote peer's ack.
 */
void process_acks(Connection &conn, u32 ack_base, u32 ack_bits)
{
    conn.sent_buffer.erase(std::remove_if(conn.sent_buffer.begin(), conn.sent_buffer.end(),
                               [=](const SentPacket &sent_packet) {
                                   if (sent_packet.sequence <= ack_base) {
                                       return true;
                                   }
                                   if (sent_packet.sequence > ack_base && sent_packet.sequence <= ack_base + 32) {
                                       u32 diff = sent_packet.sequence - ack_base;
                                       if ((ack_bits >> (diff - 1)) & 1) {
                                           return true;
                                       }
                                   }
                                   return false;
                               }),
        conn.sent_buffer.end());
}

/* --- ECS Systems --- */

/**
 * @brief Handles NetworkConnectEvent to establish a new network connection.
 */
static void network_connect_system(ecs::ResMut<Connection> conn, ecs::EventReader<NetworkConnectEvent> connect_events,
    ecs::EventWriter<NetworkErrorEvent> error_writer)
{
    for (const auto &evt : connect_events) {
        if (conn.ptr->connected) {
            r::Logger::warn("Network connect request ignored: already connected.");
            continue;
        }

        /* --- Reset Reliability State on New Connection --- */
        conn.ptr->local_sequence = 0;
        conn.ptr->remote_sequence = 0;
        conn.ptr->ack_bits = 0;
        conn.ptr->sent_buffer.clear();

        conn.ptr->protocol = evt.protocol;
        conn.ptr->endpoint = evt.endpoint;
        int sock_type = (evt.protocol == Protocol::UDP) ? SOCK_DGRAM : SOCK_STREAM;
        conn.ptr->handle = socket(AF_INET, sock_type, 0);

        if (conn.ptr->handle == -1) {
            error_writer.send({"Failed to create socket."});
            continue;
        }

        if (evt.protocol == Protocol::TCP) {
            sockaddr_in addr = {};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(evt.endpoint.port);
            inet_pton(AF_INET, evt.endpoint.address.c_str(), &addr.sin_addr);

            if (::connect(conn.ptr->handle, (sockaddr *) &addr, sizeof(addr)) < 0) {
                error_writer.send({"Failed to connect TCP socket."});
                close(conn.ptr->handle);
                conn.ptr->handle = -1;
                continue;
            }
        }

        conn.ptr->connected = true;
        r::Logger::info("Network connection established.");
    }
}

/**
 * @brief Handles NetworkDisconnectEvent to gracefully close the connection.
 */
static void network_disconnect_system(ecs::ResMut<Connection> conn, ecs::EventReader<NetworkDisconnectEvent> disconnect_events)
{
    if (disconnect_events.begin() == disconnect_events.end()) {
        return;///< No events to process
    }

    if (conn.ptr->connected && conn.ptr->handle != -1) {
        close(conn.ptr->handle);
        conn.ptr->handle = -1;
        conn.ptr->connected = false;
        r::Logger::info("Network connection closed.");
    }
}

/**
 * @brief Handles sending network packets requested via NetworkSendEvent.
 */
static void network_send_system(ecs::ResMut<Connection> conn, ecs::EventReader<NetworkSendEvent> send_events,
    ecs::EventWriter<NetworkErrorEvent> error_writer, ecs::Res<core::FrameTime> time)
{
    if (!conn.ptr->connected || conn.ptr->handle == -1)
        return;

    bool is_reliable = conn.ptr->protocol == Protocol::UDP;

    for (const auto &evt : send_events) {
        Packet packet_to_send = evt.packet;

        if (is_reliable) {
            conn.ptr->local_sequence++;
            packet_to_send.sequence = conn.ptr->local_sequence;
            packet_to_send.ackBase = conn.ptr->remote_sequence;
            packet_to_send.ackBits = conn.ptr->ack_bits;
        }

        std::vector<u8> buffer = serializePacket(packet_to_send);
        ssize_t sent_bytes = 0;

        if (conn.ptr->protocol == Protocol::TCP) {
            sent_bytes = ::send(conn.ptr->handle, buffer.data(), buffer.size(), 0);
        } else {///< UDP
            sockaddr_in addr = {};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(conn.ptr->endpoint.port);
            inet_pton(AF_INET, conn.ptr->endpoint.address.c_str(), &addr.sin_addr);
            sent_bytes = ::sendto(conn.ptr->handle, buffer.data(), buffer.size(), 0, (sockaddr *) &addr, sizeof(addr));
        }

        if (sent_bytes < 0) {
            error_writer.send({"Network send error."});
        } else if (is_reliable) {
            conn.ptr->sent_buffer.push_back({time.ptr->global_time, packet_to_send.sequence, buffer});
        }
    }
}

/**
 * @brief Polls the active connection for incoming data and fires NetworkMessageEvent.
 */
static void network_receive_system(ecs::ResMut<Connection> conn, ecs::EventWriter<NetworkMessageEvent> message_writer,
    ecs::EventWriter<NetworkErrorEvent> error_writer)
{
    if (!conn.ptr->connected || conn.ptr->handle == -1)
        return;

    std::vector<u8> buffer(2048);
    ssize_t received = ::recv(conn.ptr->handle, buffer.data(), buffer.size(), MSG_DONTWAIT);

    if (received > 0) {
        buffer.resize(static_cast<size_t>(received));
        Packet packet = deserializePacket(buffer);

        if (conn.ptr->protocol == Protocol::UDP) {
            process_acks(*conn.ptr, packet.ackBase, packet.ackBits);
            process_incoming_sequence(*conn.ptr, packet.sequence);
        }

        message_writer.send({packet.command, packet.payload});
    } else if (received == 0 && conn.ptr->protocol == Protocol::TCP) {
        /* TCP connection closed by peer */
        r::Logger::info("Peer closed the connection.");
        close(conn.ptr->handle);
        conn.ptr->handle = -1;
        conn.ptr->connected = false;
    } else if (received < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        error_writer.send({"Network receive error."});
        close(conn.ptr->handle);
        conn.ptr->handle = -1;
        conn.ptr->connected = false;
    }
}

/**
 * @brief Handles retransmitting lost packets for UDP connections.
 */
static void network_resend_system(ecs::ResMut<Connection> conn, ecs::Res<core::FrameTime> time,
    ecs::EventWriter<NetworkErrorEvent> error_writer)
{
    if (!conn.ptr->connected || conn.ptr->handle == -1 || conn.ptr->protocol != Protocol::UDP) {
        return;
    }

    for (auto &sent_packet : conn.ptr->sent_buffer) {
        if (time.ptr->global_time - sent_packet.sent_time > conn.ptr->timeout_seconds) {
            sockaddr_in addr = {};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(conn.ptr->endpoint.port);
            inet_pton(AF_INET, conn.ptr->endpoint.address.c_str(), &addr.sin_addr);

            ssize_t sent_bytes =
                ::sendto(conn.ptr->handle, sent_packet.buffer.data(), sent_packet.buffer.size(), 0, (sockaddr *) &addr, sizeof(addr));

            if (sent_bytes < 0) {
                error_writer.send({"Network resend error."});
            } else {
                sent_packet.sent_time = time.ptr->global_time;
                r::Logger::debug("Retransmitted packet with sequence: " + std::to_string(sent_packet.sequence));
            }
        }
    }
}

}// namespace

/* --- Plugin Implementation --- */

void NetworkPlugin::build(Application &app)
{
    app.insert_resource(Connection{})
        .add_events<NetworkConnectEvent, NetworkDisconnectEvent, NetworkSendEvent, NetworkMessageEvent, NetworkErrorEvent>()
        .add_systems<network_connect_system, network_disconnect_system, network_send_system, network_receive_system, network_resend_system>(
            Schedule::UPDATE);

    r::Logger::debug("NetworkPlugin built");
}

}// namespace r::net

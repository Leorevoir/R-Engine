/**
 * @file NetworkPlugin.hpp
 * @brief Defines the network plugin, its components, and related data structures for the R-Engine.
 */

#pragma once

#include "R-Engine/Application.hpp"
#include "R-Engine/Plugins/Plugin.hpp"
#include "R-Engine/Types.hpp"
#include <RTypeNet/Interfaces.hpp>
#include <string>
#include <vector>

namespace r::net {

/**
 * @brief Defines the supported network protocols.
 */
enum class Protocol {
    UDP,///< User Datagram Protocol.
    TCP ///< Transmission Control Protocol.
};

/**
 * @brief Represents a network endpoint with an IP address and port.
 */
struct Endpoint {
        std::string address;
        u16 port;
};

/**
 * @brief Represents a packet that has been sent but not yet acknowledged.
 * @details Stores the serialized packet data and the time it was sent to manage retransmissions.
 */
struct SentPacket {
        f32 sent_time;         ///< The global time the packet was sent.
        u32 sequence;          ///< The sequence number of the packet.
        std::vector<u8> buffer;///< The raw serialized packet data.
};

/**
 * @brief ECS resource representing the state of a single network connection.
 * @details This resource holds the socket handle and connection status. Systems interact
 * with this resource to perform network operations.
 */
struct Connection {
        rtype::network::Socket socket{};
        bool connected = false;

        /* --- Reliability State (for UDP) --- */

        /* Outgoing packet state */
        u32 local_sequence = 0;             ///< Sequence number for the next outgoing packet.
        std::vector<SentPacket> sent_buffer;///< Buffer of sent packets awaiting acknowledgment.

        /* Incoming packet state */
        u32 remote_sequence = 0;///< Highest sequence number received from the remote peer.
        u32 ack_bits = 0;       ///< Bitfield of acknowledged packets beyond the remote_sequence.

        /* Configuration */
        f32 rtt = 0.0f;            ///< Round-trip time estimate (not yet implemented).
        f32 timeout_seconds = 1.0f;///< Time before a packet is considered lost and retransmitted.
};

/**
 * @brief Event to request a network connection. Systems listen for this event to initiate a connection.
 */
struct NetworkConnectEvent {
        Endpoint endpoint;
        Protocol protocol;
};

/**
 * @brief Event to request a network disconnection.
 */
struct NetworkDisconnectEvent {
};

/**
 * @brief Event fired when a network error occurs.
 */
struct NetworkErrorEvent {
        std::string message;
};

/**
 * @brief Represents a binary data packet for network communication.
 */
struct Packet {
        u16 magic;
        u8 version;
        u8 flags;
        u32 sequence;
        u32 ackBase;
        u32 ackBits;
        u8 channel;
        u16 size;
        u32 clientId;
        u8 command;
        std::vector<u8> payload;
};

/**
 * @brief Event to request sending a packet over the network.
 * @details Other systems create this event to have the networking systems
 * serialize and transmit the packet.
 */
struct NetworkSendEvent {
        Packet packet;
};

/**
 * @brief Generic event fired upon receiving any network message.
 * @details This event contains the raw message type and payload from a deserialized packet.
 */
struct NetworkMessageEvent {
        u8 message_type;
        std::vector<u8> payload;
};

/**
 * @brief The main plugin for integrating network functionalities into the R-Engine application.
 *
 * This plugin sets up all the necessary ECS systems and resources for handling network connections,
 * events, and data transmission in a way that is consistent with the engine's architecture.
 */
class NetworkPlugin final : public Plugin
{
    public:
        NetworkPlugin() noexcept = default;
        ~NetworkPlugin() override = default;

        /**
     * @brief Builds the plugin, adding necessary ECS resources, events, and systems to the app.
     * @param app A reference to the Application.
     */
        void build(Application &app) override;
};

}// namespace r::net

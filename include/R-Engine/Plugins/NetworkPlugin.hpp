#pragma once
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <functional>

namespace r::net {

/**
 * @brief Supported network protocols.
 */
enum class Protocol {
    UDP,
    TCP
};

/**
 * @brief Represents an IP address and port.
 */
struct Endpoint {
    std::string address;
    uint16_t port;
};

/**
 * @brief Manages the socket, connection state, etc. Stored as an ECS Resource.
 */
struct Connection {
    Protocol protocol = Protocol::TCP;
    int handle = -1;
    bool connected = false;
    Endpoint endpoint;
};

/**
 * @brief Event for network connection.
 */
struct NetworkConnectEvent {
    Endpoint endpoint;
    Protocol protocol;
};

/**
 * @brief Event for network disconnection.
 */
struct NetworkDisconnectEvent {};

/**
 * @brief Event for network errors.
 */
struct NetworkErrorEvent {
    std::string message;
};

/**
 * @brief Binary packet structure compatible with R-Type server protocol.
 */
struct Packet {
    uint16_t magic;
    uint8_t version;
    uint8_t flags;
    uint32_t sequence;
    uint32_t ackBase;
    uint8_t ackBits;
    uint8_t channel;
    uint16_t size;
    uint32_t clientId;
    uint8_t command;
    std::vector<uint8_t> payload;
    Packet() = default;
};

/**
 * @brief A generic event fired when any message is received from the server.
 */
struct NetworkMessageEvent {
    uint8_t message_type;
    std::vector<uint8_t> payload;
};

class NetworkPlugin final : public Plugin {
public:
    NetworkPlugin() noexcept = default;
    ~NetworkPlugin() override = default;
    void build(Application &app) override;
};

} /* namespace r::net */

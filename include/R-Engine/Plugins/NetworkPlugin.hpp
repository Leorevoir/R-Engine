/**
 * @file NetworkPlugin.hpp
 * @brief Defines the network plugin, its components, and related data structures for the R-Engine.
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>

#include "R-Engine/Application.hpp"
#include "R-Engine/Plugins/Plugin.hpp"

namespace r::net {

/**
 * @brief Defines the supported network protocols.
 */
enum class Protocol {
    UDP, ///< User Datagram Protocol.
    TCP  ///< Transmission Control Protocol.
};

/**
 * @brief Represents a network endpoint with an IP address and port.
 */
struct Endpoint {
    std::string address;
    uint16_t port;
};

/**
 * @brief ECS resource representing the state of a single network connection.
 */
struct Connection {
    Protocol protocol = Protocol::TCP;
    int handle = -1;
    bool connected = false;
    Endpoint endpoint;
};

/**
 * @brief Event to request a network connection.
 */
struct NetworkConnectEvent {
    Endpoint endpoint;
    Protocol protocol;
};

/**
 * @brief Event to request a network disconnection.
 */
struct NetworkDisconnectEvent {};

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
 * @brief Generic event fired upon receiving any network message.
 */
struct NetworkMessageEvent {
    uint8_t message_type;
    std::vector<uint8_t> payload;
};

/**
 * @brief A low-level wrapper around a system socket (TCP or UDP).
 */
class Socket {
public:
    /**
     * @brief Constructs a new Socket object.
     * @param socketProtocol The network protocol (TCP or UDP) to be used.
     */
    Socket(Protocol socketProtocol);

    /**
     * @brief Destroys the Socket object and ensures the connection is closed.
     */
    ~Socket();

    /**
     * @brief Connects (for TCP) or binds (for UDP) the socket to a specific endpoint.
     * @param endpoint The remote or local endpoint to connect/bind to.
     */
    void connect(const Endpoint &endpoint);

    /**
     * @brief Closes the socket connection.
     */
    void disconnect();

    /**
     * @brief Sends a raw byte buffer over the socket.
     * @param data The vector of bytes to send.
     * @param endpoint The destination endpoint (primarily for UDP).
     * @return The number of bytes successfully sent.
     */
    size_t send(const std::vector<uint8_t>& data, const Endpoint* endpoint = nullptr);

    /**
     * @brief Receives raw bytes from the socket into a buffer.
     * @param buffer The vector to store the received data.
     * @param endpoint A pointer to store the sender's endpoint (primarily for UDP).
     * @return The number of bytes successfully received.
     */
    size_t recv(std::vector<uint8_t>& buffer, Endpoint* endpoint = nullptr);

    /**
     * @brief Checks if the socket is currently connected.
     * @return True if the socket is connected, false otherwise.
     */
    bool isConnected() const;

private:
    void configureSocket();
    int handle;
    Protocol protocol;
    bool connected;
};

/**
 * @brief Serializes a Packet struct into a byte vector for transmission.
 * @param packet The Packet object to serialize.
 * @return A std::vector<uint8_t> containing the serialized data.
 */
std::vector<uint8_t> serializePacket(const Packet& packet);

/**
 * @brief Deserializes a byte vector into a Packet struct.
 * @param buffer The byte vector to deserialize.
 * @return The deserialized Packet object.
 */
Packet deserializePacket(const std::vector<uint8_t>& buffer);


/**
 * @brief The main plugin for integrating network functionalities into the R-Engine application.
 *
 * This plugin sets up ECS systems and resources for handling network connections,
 * events, and data transmission.
 */
class NetworkPlugin final : public Plugin {
public:
    NetworkPlugin() noexcept;
    ~NetworkPlugin() override;

    /**
     * @brief Builds the plugin, adding necessary ECS resources, events, and systems to the app.
     * @param app A reference to the Application.
     */
    void build(Application &app) override;

    /**
     * @brief Initiates a connection to a server.
     * @param serverEndpoint The endpoint of the server to connect to.
     * @param protocol The protocol to use for the connection.
     */
    void connectToServer(const Endpoint& serverEndpoint, Protocol protocol);

    /**
     * @brief Disconnects from the currently connected server.
     */
    void disconnectFromServer();

    /**
     * @brief Serializes and sends a Packet over the network.
     * @param packet The Packet to send.
     * @param protocol The protocol to use.
     * @param endpoint The destination endpoint (for UDP).
     */
    void sendPacket(const Packet& packet, Protocol protocol, const Endpoint* endpoint = nullptr);

    /**
     * @brief Receives data and deserializes it into a Packet.
     * @param protocol The protocol to receive on.
     * @param endpoint Pointer to store the sender's endpoint.
     * @return The received Packet.
     */
    Packet receivePacket(Protocol protocol, Endpoint* endpoint = nullptr);

    /**
     * @brief Sets a callback function to be invoked on network errors.
     * @param callback The function to call.
     */
    void setNetworkErrorCallback(std::function<void(const std::string&)> callback);

    /**
     * @brief Sets a callback function to be invoked upon successful reconnection.
     * @param callback The function to call.
     */
    void setReconnectCallback(std::function<void()> callback);

    /**
     * @brief Attempts to automatically reconnect to a server.
     * @param serverEndpoint The endpoint of the server.
     * @param protocol The protocol to use.
     */
    void reconnectToServer(const Endpoint& serverEndpoint, Protocol protocol);

    /**
     * @brief Sends a raw byte buffer over the TCP socket.
     * @param buffer The data to send.
     * @param endpoint The destination endpoint.
     */
    void sendRawTcp(const std::vector<uint8_t> &buffer, const Endpoint &endpoint);

    /**
     * @brief Receives a raw byte buffer from the TCP socket.
     * @param buffer The buffer to store received data.
     * @param endpoint Pointer to store sender information.
     */
    void recvRawTcp(std::vector<uint8_t> &buffer, Endpoint *endpoint);

private:
    void startNetworkThread();
    void stopNetworkThread();
    void resolveServerConflict(const std::vector<uint8_t>& serverData, std::vector<uint8_t>& clientData);
    uint64_t now_ns();

    std::unique_ptr<Socket> tcpSocket;          ///< Smart pointer to the TCP socket handler.
    std::unique_ptr<Socket> udpSocket;          ///< Smart pointer to the UDP socket handler.

    std::thread networkThread;                  ///< Background thread for handling network operations.
    std::atomic<bool> networkThreadRunning{false}; ///< Atomic flag to control the network thread's lifecycle.

    Endpoint pendingTcpEndpoint;                ///< Endpoint for pending TCP connection attempts.
    Endpoint pendingUdpEndpoint;                ///< Endpoint for pending UDP connection attempts.

    std::function<void(const std::string&)> errorCallback; ///< Callback for network errors.
    std::function<void()> reconnectCallback;    ///< Callback for successful reconnections.
};

} // namespace r::net
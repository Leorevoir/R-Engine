#ifndef NETWORK_PLUGIN_HPP
#define NETWORK_PLUGIN_HPP

#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <thread>
#include <functional>


namespace rtype::network {

// Supported protocols
enum class Protocol {
    UDP,
    TCP
};

// Supported message types for communication
enum class MessageType {
    CONNECT,
    DISCONNECT,
    GAME_UPDATE,
    COMMAND
};

// Represents an IP address and port
struct Endpoint {
    std::string address;
    uint16_t port;
};


// Socket TCP/UDP
class Socket {
public:
    Socket(Protocol protocol);
    ~Socket();

    void connect(const Endpoint& endpoint);
    void disconnect();

    size_t send(const std::vector<uint8_t>& data, const Endpoint* endpoint = nullptr);
    size_t recv(std::vector<uint8_t>& buffer, Endpoint* senderEndpoint = nullptr);

private:
    Protocol protocol;
    int handle;
    void configureSocket();
};

// Structure de paquet binaire compatible serveur R-Type
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

// Main NetworkPlugin class
class NetworkPlugin {
public:
    void sendRawTcp(const std::vector<uint8_t>& buffer, const Endpoint& endpoint);
    void recvRawTcp(std::vector<uint8_t>& buffer, Endpoint* endpoint = nullptr);
public:
    NetworkPlugin();
    ~NetworkPlugin();

    void connectToServer(const Endpoint& serverEndpoint, Protocol protocol = Protocol::TCP);
    void disconnectFromServer();
    void reconnectToServer(const Endpoint& serverEndpoint, Protocol protocol = Protocol::TCP);

    void sendPacket(const Packet& packet, Protocol protocol = Protocol::TCP, const Endpoint* endpoint = nullptr);
    Packet receivePacket(Protocol protocol = Protocol::TCP, Endpoint* senderEndpoint = nullptr);

    void resolveServerConflict(const std::vector<uint8_t>& serverData, std::vector<uint8_t>& clientData);
    uint64_t now_ns();

    void startNetworkThread(const Endpoint& serverEndpoint, Protocol protocol = Protocol::TCP);
    void stopNetworkThread();

    // Callbacks
    void setNetworkErrorCallback(std::function<void(const std::string&)> callback);
    void setReconnectCallback(std::function<void()> callback);

private:
    std::unique_ptr<Socket> tcpSocket;
    std::unique_ptr<Socket> udpSocket;
    std::thread networkThread;
    std::function<void(const std::string&)> onNetworkError;
    std::function<void()> onReconnect;
};

// Fonctions de sérialisation/désérialisation binaire
std::vector<uint8_t> serializePacket(const Packet& packet);
Packet deserializePacket(const std::vector<uint8_t>& buffer);

// Fonctions utilitaires
std::string generateUniqueId();
std::vector<uint8_t> encryptData(const std::vector<uint8_t>& data, const std::string& key);
std::vector<uint8_t> decryptData(const std::vector<uint8_t>& encryptedData, const std::string& key);
std::string generateEncryptionKey();
std::string extractEncryptionKey(const std::vector<uint8_t>& buffer);
std::vector<uint8_t> extractEncryptedData(const std::vector<uint8_t>& buffer);

} // namespace rtype::network

#endif // NETWORK_PLUGIN_HPP
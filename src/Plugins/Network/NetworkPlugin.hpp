
#pragma once

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

/**
 * @brief Supported network protocols.
 */
enum class Protocol {
    UDP,
    TCP
};

/**
 * @brief Supported message types for communication.
 */
enum class MessageType {
    CONNECT,
    DISCONNECT,
    GAME_UPDATE,
    COMMAND
};

/**
 * @brief Represents an IP address and port.
 */
struct Endpoint {
    std::string address;
    uint16_t port;
};


/**
 * @brief TCP/UDP socket abstraction for network communication.
 */
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

    void setNetworkErrorCallback(std::function<void(const std::string&)> callback);
    void setReconnectCallback(std::function<void()> callback);

private:
    /**
     * @brief Main network plugin class for R-Type protocol communication.
     */
    std::unique_ptr<Socket> tcpSocket;
    std::unique_ptr<Socket> udpSocket;
    std::thread networkThread;
    std::function<void(const std::string&)> onNetworkError;
    std::function<void()> onReconnect;
};

/**
 * @brief Serialize a Packet to binary format.
 * @param packet Packet to serialize.
 * @return Serialized byte vector.
 */
std::vector<uint8_t> serializePacket(const Packet& packet);
/**
 * @brief Deserialize a Packet from binary format.
 * @param buffer Byte buffer to deserialize.
 * @return Deserialized Packet.
 */
Packet deserializePacket(const std::vector<uint8_t>& buffer);

/**
 * @brief Generate a unique identifier string.
 * @return Unique ID string.
 */
std::string generateUniqueId();
/**
 * @brief Encrypt data using a key.
 * @param data Data to encrypt.
 * @param key Encryption key.
 * @return Encrypted byte vector.
 */
std::vector<uint8_t> encryptData(const std::vector<uint8_t>& data, const std::string& key);
/**
 * @brief Decrypt data using a key.
 * @param encryptedData Data to decrypt.
 * @param key Encryption key.
 * @return Decrypted byte vector.
 */
std::vector<uint8_t> decryptData(const std::vector<uint8_t>& encryptedData, const std::string& key);
/**
 * @brief Generate a random encryption key.
 * @return Encryption key string.
 */
std::string generateEncryptionKey();
/**
 * @brief Extract encryption key from a buffer.
 * @param buffer Byte buffer containing key.
 * @return Extracted key string.
 */
std::string extractEncryptionKey(const std::vector<uint8_t>& buffer);
/**
 * @brief Extract encrypted data from a buffer.
 * @param buffer Byte buffer containing encrypted data.
 * @return Extracted encrypted byte vector.
 */
std::vector<uint8_t> extractEncryptedData(const std::vector<uint8_t>& buffer);

} // namespace rtype::network

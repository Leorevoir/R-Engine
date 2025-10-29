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

namespace r::net {
{
/**
 * @brief Supported network protocols.
 */
enum class Protocol
{
    UDP,
    TCP
};
/**
 * @brief Supported message types for communication.
 */
enum class MessageType
{
    CONNECT,
    DISCONNECT,
    GAME_UPDATE,
    COMMAND
};
/**
 * @brief Represents an IP address and port.
 */
struct Endpoint
{
    std::string address;
    uint16_t port;
};
// Manages the socket, connection state, etc. Stored as an ECS Resource.
struct Connection {
    Protocol protocol = Protocol::TCP;
    int handle = -1;
    bool connected = false;
    Endpoint endpoint;
    // ... autres membres nécessaires ...
struct NetworkConnectEvent {
    Endpoint endpoint;
    Protocol protocol;
};

struct NetworkDisconnectEvent {};

struct NetworkErrorEvent {
    std::string message;
};
};
/**
 * @brief Binary packet structure compatible with R-Type server protocol.
 */
struct Packet
{
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

// A generic event fired when any message is received from the server.
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

} /* namespace rtype::network */

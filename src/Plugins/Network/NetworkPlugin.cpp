#include "NetworkPlugin.hpp"
#include <vector>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <chrono>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iomanip>
#include <sstream>
#include <thread>
#include <atomic>
#include <functional>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#define INVALID_SOCKET -1
#endif

namespace rtype::network {

void NetworkPlugin::sendRawTcp(const std::vector<uint8_t>& buffer, const Endpoint& endpoint) {
    if (tcpSocket)
        tcpSocket->send(buffer, &endpoint);
}

void NetworkPlugin::recvRawTcp(std::vector<uint8_t>& buffer, Endpoint* endpoint) {
    if (tcpSocket)
        tcpSocket->recv(buffer, endpoint);
}



namespace {
    enum class LogLevel {
        INFO,
        WARNING,
        ERROR
    };

    void logMessage(LogLevel level, const std::string& message) {
        const char* levelStr = nullptr;
        switch (level) {
            case LogLevel::INFO:
                levelStr = "[INFO]";
                break;
            case LogLevel::WARNING:
                levelStr = "[WARNING]";
                break;
            case LogLevel::ERROR:
                levelStr = "[ERROR]";
                break;
            default:
                levelStr = "[UNKNOWN]";
                break;
        }
        std::cout << levelStr << " " << message << std::endl;
    }
}

Socket::Socket(Protocol socketProtocol) : protocol(socketProtocol), handle(-1) {
    logMessage(LogLevel::INFO, "Creating socket...");
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        logMessage(LogLevel::ERROR, "Failed to initialize Winsock.");
        throw std::runtime_error("Failed to initialize Winsock.");
    }
#endif

    if (protocol == Protocol::UDP) {
        handle = socket(AF_INET, SOCK_DGRAM, 0);
        if (handle == INVALID_SOCKET) {
            logMessage(LogLevel::ERROR, "Failed to create UDP socket.");
            throw std::runtime_error("Failed to create UDP socket.");
        }
    } else {
        handle = socket(AF_INET, SOCK_STREAM, 0);
        if (handle == INVALID_SOCKET) {
            logMessage(LogLevel::ERROR, "Failed to create TCP socket.");
            throw std::runtime_error("Failed to create TCP socket.");
        }
    }
    logMessage(LogLevel::INFO, "Socket created successfully.");
}

Socket::~Socket() {
    disconnect();
#ifdef _WIN32
    WSACleanup();
#endif
}

void Socket::configureSocket() {
    if (handle == -1) {
        std::cerr << "Socket handle is invalid." << std::endl;
        return;
    }

    int opt = 1;
    if (setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Failed to set SO_REUSEADDR option." << std::endl;
    }

    std::cout << "Socket configured with SO_REUSEADDR." << std::endl;
}

void Socket::connect(const Endpoint& endpoint) {
    try {
        std::cout << "Connecting to " << endpoint.address << ":" << endpoint.port << std::endl;
        configureSocket();

        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(endpoint.port);
        if (inet_pton(AF_INET, endpoint.address.c_str(), &addr.sin_addr) <= 0) {
            throw std::system_error(std::make_error_code(std::errc::address_not_available), "Invalid endpoint address");
        }

        if (protocol == Protocol::UDP) {
            if (bind(handle, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
                throw std::runtime_error("Failed to bind UDP socket.");
            }
        } else {
            if (::connect(handle, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
                throw std::runtime_error("Failed to connect TCP socket.");
            }
        }
    } catch (const std::system_error& e) {
        std::cerr << "Connection error: " << e.what() << std::endl;
        throw;
    }
}

void Socket::disconnect() {
    if (handle != -1) {
        logMessage(LogLevel::INFO, "Disconnecting socket...");
#ifdef _WIN32
        closesocket(handle);
#else
        close(handle);
#endif
        handle = -1;
        logMessage(LogLevel::INFO, "Socket disconnected.");
    }
}

size_t Socket::send(const std::vector<uint8_t>& data, const Endpoint* endpoint) {
    try {
        if (data.empty()) {
            throw std::system_error(std::make_error_code(std::errc::invalid_argument), "Cannot send empty data");
        }
        std::cout << "Sending data of size: " << data.size() << std::endl;

        if (protocol == Protocol::UDP && endpoint) {
            sockaddr_in addr = {};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(endpoint->port);
            if (inet_pton(AF_INET, endpoint->address.c_str(), &addr.sin_addr) <= 0) {
                throw std::runtime_error("Invalid endpoint address for UDP send.");
            }
            ssize_t sent = sendto(handle, data.data(), data.size(), 0, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
            return sent >= 0 ? static_cast<size_t>(sent) : 0;
        } else {
            ssize_t sent = ::send(handle, data.data(), data.size(), 0);
            return sent >= 0 ? static_cast<size_t>(sent) : 0;
        }
    } catch (const std::system_error& e) {
        std::cerr << "Send error: " << e.what() << std::endl;
        return 0;
    }
}

size_t Socket::recv(std::vector<uint8_t>& buffer, Endpoint* endpoint) {
    try {
        std::cout << "Receiving data..." << std::endl;
        buffer.resize(1024);

        if (protocol == Protocol::UDP && endpoint) {
            sockaddr_in addr = {};
            socklen_t addrLen = sizeof(addr);
            ssize_t received = recvfrom(handle, buffer.data(), buffer.size(), 0, reinterpret_cast<sockaddr*>(&addr), &addrLen);
            if (received >= 0) {
                buffer.resize(static_cast<size_t>(received));
                char addrStr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &addr.sin_addr, addrStr, sizeof(addrStr));
                endpoint->address = addrStr;
                endpoint->port = ntohs(addr.sin_port);
                return static_cast<size_t>(received);
            }
            return 0;
        } else {
            ssize_t received = ::recv(handle, buffer.data(), buffer.size(), 0);
            if (received >= 0) {
                buffer.resize(static_cast<size_t>(received));
                return static_cast<size_t>(received);
            }
            return 0;
        }
    } catch (const std::system_error& e) {
        std::cerr << "Receive error: " << e.what() << std::endl;
        return 0;
    }
}

NetworkPlugin::NetworkPlugin() : tcpSocket(nullptr), udpSocket(nullptr) {
    std::cout << "NetworkPlugin initialized." << std::endl;
}

NetworkPlugin::~NetworkPlugin() {
    disconnectFromServer();
}

void NetworkPlugin::connectToServer(const Endpoint& serverEndpoint, Protocol protocol) {
    if (protocol == Protocol::TCP) {
        tcpSocket = std::make_unique<Socket>(protocol);
        tcpSocket->connect(serverEndpoint);
    } else {
        udpSocket = std::make_unique<Socket>(protocol);
        udpSocket->connect(serverEndpoint);
    }
}

void NetworkPlugin::disconnectFromServer() {
    if (tcpSocket) {
        tcpSocket->disconnect();
        tcpSocket.reset();
    }
    if (udpSocket) {
        udpSocket->disconnect();
        udpSocket.reset();
    }
}

void NetworkPlugin::setNetworkErrorCallback(std::function<void(const std::string&)> callback) {
    onNetworkError = std::move(callback);
}

void NetworkPlugin::setReconnectCallback(std::function<void()> callback) {
    onReconnect = std::move(callback);
}

void NetworkPlugin::reconnectToServer(const Endpoint& serverEndpoint, Protocol protocol) {
    int retryCount = 0;
    const int maxRetries = 5;
    const int retryDelayMs = 1000; // 1 second

    while (retryCount < maxRetries) {
        try {
            logMessage(LogLevel::INFO, "Attempting to reconnect (attempt " + std::to_string(retryCount + 1) + ")...");
            connectToServer(serverEndpoint, protocol);
            logMessage(LogLevel::INFO, "Reconnection successful.");
            if (onReconnect) onReconnect();
            return;
        } catch (const std::exception& e) {
            logMessage(LogLevel::ERROR, std::string("Reconnection attempt failed: ") + e.what());
            if (onNetworkError) onNetworkError(e.what());
            ++retryCount;
            std::this_thread::sleep_for(std::chrono::milliseconds(retryDelayMs));
        }
    }
    if (onNetworkError) onNetworkError("Failed to reconnect after " + std::to_string(maxRetries) + " attempts.");
    throw std::runtime_error("Failed to reconnect after " + std::to_string(maxRetries) + " attempts.");
}


// Sérialisation d'un Packet en buffer binaire
std::vector<uint8_t> serializePacket(const Packet& packet) {
    std::vector<uint8_t> buffer;
    buffer.resize(2 + 1 + 1 + 4 + 4 + 1 + 1 + 4 + 4 + 1); // header size (size sur 4 octets)
    size_t offset = 0;
    auto write16 = [&](uint16_t v) { v = htons(v); memcpy(&buffer[offset], &v, 2); offset += 2; };
    auto write32 = [&](uint32_t v) { v = htonl(v); memcpy(&buffer[offset], &v, 4); offset += 4; };
    auto write8 = [&](uint8_t v) { buffer[offset++] = v; };

    write16(packet.magic);
    write8(packet.version);
    write8(packet.flags);
    write32(packet.sequence);
    write32(packet.ackBase);
    write8(packet.ackBits);
    write8(packet.channel);
    write32(static_cast<uint32_t>(packet.size)); // size sur 4 octets
    write32(packet.clientId);
    write8(packet.command);

    buffer.insert(buffer.end(), packet.payload.begin(), packet.payload.end());
    return buffer;
}

Packet deserializePacket(const std::vector<uint8_t>& buffer) {
    Packet packet;
    size_t offset = 0;
    auto read16 = [&]() { uint16_t v; memcpy(&v, &buffer[offset], 2); offset += 2; return ntohs(v); };
    auto read32 = [&]() { uint32_t v; memcpy(&v, &buffer[offset], 4); offset += 4; return ntohl(v); };
    auto read8 = [&]() { return buffer[offset++]; };

    packet.magic = read16();
    packet.version = read8();
    packet.flags = read8();
    packet.sequence = read32();
    packet.ackBase = read32();
    packet.ackBits = read8();
    packet.channel = read8();
    packet.size = read16();
    packet.clientId = read32();
    packet.command = read8();
    if (buffer.size() > offset)
        packet.payload = std::vector<uint8_t>(buffer.begin() + static_cast<std::vector<uint8_t>::difference_type>(offset), buffer.end());
    return packet;
}

std::string generateUniqueId() {
    static uint64_t counter = 0;
    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << ++counter;
    return oss.str();
}

std::vector<uint8_t> encryptData(const std::vector<uint8_t>& data, const std::string& key) {
    std::vector<uint8_t> encryptedData(data.size() + EVP_MAX_BLOCK_LENGTH);
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    if (!ctx) {
        throw std::runtime_error("Failed to create encryption context.");
    }

    int len;
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char*>(key.data()), nullptr) != 1 ||
        EVP_EncryptUpdate(ctx, encryptedData.data(), &len, data.data(), static_cast<int>(data.size())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Encryption failed.");
    }

    int finalLen;
    if (EVP_EncryptFinal_ex(ctx, encryptedData.data() + len, &finalLen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Final encryption step failed.");
    }

    encryptedData.resize(static_cast<size_t>(len + finalLen));
    EVP_CIPHER_CTX_free(ctx);
    return encryptedData;
}

std::vector<uint8_t> decryptData(const std::vector<uint8_t>& encryptedData, const std::string& key) {
    std::vector<uint8_t> decryptedData(encryptedData.size());
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    if (!ctx) {
        throw std::runtime_error("Failed to create decryption context.");
    }

    int len;
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char*>(key.data()), nullptr) != 1 ||
        EVP_DecryptUpdate(ctx, decryptedData.data(), &len, encryptedData.data(), static_cast<int>(encryptedData.size())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Decryption failed.");
    }

    int finalLen;
    if (EVP_DecryptFinal_ex(ctx, decryptedData.data() + len, &finalLen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Final decryption step failed.");
    }

    decryptedData.resize(static_cast<size_t>(len + finalLen));
    EVP_CIPHER_CTX_free(ctx);
    return decryptedData;
}

std::string generateEncryptionKey() {
    std::vector<unsigned char> key(32); // 256-bit key
    if (!RAND_bytes(key.data(), static_cast<int>(key.size()))) {
        throw std::runtime_error("Failed to generate encryption key.");
    }
    return std::string(key.begin(), key.end());
}


void NetworkPlugin::sendPacket(const Packet& packet, Protocol protocol, const Endpoint* endpoint) {
    auto buffer = serializePacket(packet);
    if (protocol == Protocol::TCP && tcpSocket) {
        tcpSocket->send(buffer, endpoint);
    } else if (protocol == Protocol::UDP && udpSocket) {
        udpSocket->send(buffer, endpoint);
    }
}



// Format de message sécurisé :
// [clé de chiffrement (32 octets)] + [données chiffrées]
// La clé est générée dynamiquement pour chaque message et doit être extraite côté serveur avant déchiffrement.

std::string extractEncryptionKey(const std::vector<uint8_t>& buffer) {
    if (buffer.size() < 32) {
        throw std::runtime_error("Received message is too short to contain an encryption key.");
    }
    return std::string(buffer.begin(), buffer.begin() + 32);
}

std::vector<uint8_t> extractEncryptedData(const std::vector<uint8_t>& buffer) {
    if (buffer.size() < 32) {
        throw std::runtime_error("Received message is too short to contain encrypted data.");
    }
    return std::vector<uint8_t>(buffer.begin() + 32, buffer.end());
}


Packet NetworkPlugin::receivePacket(Protocol protocol, Endpoint* endpoint) {
    std::vector<uint8_t> buffer;
    if (protocol == Protocol::TCP && tcpSocket) {
        tcpSocket->recv(buffer, endpoint);
        return deserializePacket(buffer);
    } else if (protocol == Protocol::UDP && udpSocket) {
        udpSocket->recv(buffer, endpoint);
        return deserializePacket(buffer);
    }
    return Packet{};
}

uint64_t NetworkPlugin::now_ns() {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()
    ).count());
}

void NetworkPlugin::resolveServerConflict(const std::vector<uint8_t>& serverData, std::vector<uint8_t>& clientData) {
    if (serverData != clientData) {
        clientData = serverData;
        std::cout << "Conflict detected: server data prioritized." << std::endl;
    } else {
        std::cout << "No conflict: data identical." << std::endl;
    }
}



namespace {
    std::atomic<bool> networkThreadRunning{false};
}



} // namespace rtype::network
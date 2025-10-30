#include "R-Engine/Plugins/NetworkPlugin.hpp"
#include "R-Engine/Core/Logger.hpp"
#include "R-Engine/Application.hpp"
#include "R-Engine/Plugins/Plugin.hpp"
#include <vector>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define INVALID_SOCKET -1

namespace r::net {

static void network_receive_system(
    ecs::ResMut<Connection> conn,
    ecs::EventWriter<NetworkMessageEvent> message_writer,
    ecs::EventWriter<NetworkErrorEvent> error_writer
) {
    if (!conn.ptr->connected || conn.ptr->handle == -1) return;
    std::vector<uint8_t> buffer(2048);
    ssize_t received = ::recv(conn.ptr->handle, buffer.data(), buffer.size(), MSG_DONTWAIT);
    if (received > 0) {
        buffer.resize(static_cast<size_t>(received));
        Packet packet = deserializePacket(buffer);
        message_writer.send({packet.command, packet.payload});
    } else if (received < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        error_writer.send({"Receive error."});
    }
}

static void network_connect_system(
    ecs::ResMut<Connection> conn,
    ecs::EventReader<NetworkConnectEvent> connect_events,
    ecs::EventWriter<NetworkErrorEvent> error_writer
) {
    for (const auto& evt : connect_events) {
        if (conn.ptr->connected) continue;
        conn.ptr->protocol = evt.protocol;
        conn.ptr->endpoint = evt.endpoint;
        conn.ptr->handle = socket(AF_INET, evt.protocol == Protocol::UDP ? SOCK_DGRAM : SOCK_STREAM, 0);
        if (conn.ptr->handle == -1) {
            error_writer.send({"Failed to create socket."});
            continue;
        }
        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(evt.endpoint.port);
        inet_pton(AF_INET, evt.endpoint.address.c_str(), &addr.sin_addr);
        int res = (evt.protocol == Protocol::UDP)
            ? bind(conn.ptr->handle, (sockaddr*)&addr, sizeof(addr))
            : ::connect(conn.ptr->handle, (sockaddr*)&addr, sizeof(addr));
        if (res < 0) {
            error_writer.send({"Failed to connect/bind socket."});
            close(conn.ptr->handle);
            conn.ptr->handle = -1;
            continue;
        }
        conn.ptr->connected = true;
    }
}

static void network_disconnect_system(
    ecs::ResMut<Connection> conn,
    [[maybe_unused]] ecs::EventReader<NetworkDisconnectEvent> disconnect_events
) {
    if (!conn.ptr->connected || conn.ptr->handle == -1) return;
    close(conn.ptr->handle);
    conn.ptr->handle = -1;
    conn.ptr->connected = false;
}

Socket::Socket(Protocol socketProtocol) : handle(-1), protocol(socketProtocol), connected(false) {
    handle = (protocol == Protocol::UDP) ? socket(AF_INET, SOCK_DGRAM, 0) : socket(AF_INET, SOCK_STREAM, 0);
    if (handle == INVALID_SOCKET) throw std::runtime_error("Failed to create socket.");
}

Socket::~Socket() { disconnect(); }

void Socket::configureSocket() {
    if (handle == -1) return;
    int opt = 1;
    setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

void Socket::connect(const Endpoint &endpoint) {
    configureSocket();
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(endpoint.port);
    if (inet_pton(AF_INET, endpoint.address.c_str(), &addr.sin_addr) <= 0) throw std::runtime_error("Invalid address");
    int res = (protocol == Protocol::UDP) ? bind(handle, (sockaddr*)&addr, sizeof(addr)) : ::connect(handle, (sockaddr*)&addr, sizeof(addr));
    if (res < 0) throw std::runtime_error("Socket connect/bind failed");
    connected = true;
}

void Socket::disconnect() {
    if (handle != -1) { close(handle); handle = -1; connected = false; }
}

size_t Socket::send(const std::vector<uint8_t>& data, const Endpoint* endpoint) {
    if (data.empty()) return 0;
    ssize_t sent = (protocol == Protocol::UDP && endpoint)
        ? sendto(handle, data.data(), data.size(), 0, nullptr, 0)
        : ::send(handle, data.data(), data.size(), 0);
    return sent > 0 ? static_cast<size_t>(sent) : 0;
}

size_t Socket::recv(std::vector<uint8_t>& buffer, Endpoint* endpoint) {
    buffer.resize(2048);
    ssize_t received = (protocol == Protocol::UDP && endpoint)
        ? recvfrom(handle, buffer.data(), buffer.size(), 0, nullptr, nullptr)
        : ::recv(handle, buffer.data(), buffer.size(), 0);
    if (received > 0) { buffer.resize(static_cast<size_t>(received)); return static_cast<size_t>(received); }
    buffer.clear(); return 0;
}

bool Socket::isConnected() const { return connected; }

NetworkPlugin::NetworkPlugin() noexcept {}
NetworkPlugin::~NetworkPlugin() {
    stopNetworkThread();
}

void NetworkPlugin::build(Application &app) {
    app.insert_resource(Connection{})
       .add_events<NetworkConnectEvent, NetworkDisconnectEvent, NetworkMessageEvent, NetworkErrorEvent>()
       .add_systems<network_connect_system, network_disconnect_system, network_receive_system>(Schedule::UPDATE);
}

void NetworkPlugin::startNetworkThread() {
    if (networkThreadRunning) return;
    networkThreadRunning = true;
    networkThread = std::thread([this]() {
        r::Logger::info("Network thread started.");
        while (networkThreadRunning) {
            try {
                if (tcpSocket && !tcpSocket->isConnected()) {
                    r::Logger::info("Network thread: attempting TCP connection...");
                    tcpSocket->connect(pendingTcpEndpoint);
                    r::Logger::info("Network thread: TCP connection successful.");
                }
                if (udpSocket && !udpSocket->isConnected()) {
                    r::Logger::info("Network thread: attempting UDP connection...");
                    udpSocket->connect(pendingUdpEndpoint);
                    r::Logger::info("Network thread: UDP connection successful.");
                }
            } catch (const std::exception& e) {
                r::Logger::error(std::string("Network thread error: ") + e.what());
                if (errorCallback) {
                    errorCallback(e.what());
                }
                networkThreadRunning = false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        r::Logger::info("Network thread stopped.");
    });
}

void NetworkPlugin::stopNetworkThread() {
    networkThreadRunning = false;
    if (networkThread.joinable()) {
        networkThread.join();
    }
}

void NetworkPlugin::connectToServer(const Endpoint& serverEndpoint, Protocol protocol) {
    stopNetworkThread();

    if (protocol == Protocol::TCP) {
        tcpSocket = std::make_unique<Socket>(protocol);
        pendingTcpEndpoint = serverEndpoint;
    } else {
        udpSocket = std::make_unique<Socket>(protocol);
        pendingUdpEndpoint = serverEndpoint;
    }

    startNetworkThread();
}

void NetworkPlugin::disconnectFromServer() {
    stopNetworkThread();
    if (tcpSocket) tcpSocket.reset();
    if (udpSocket) udpSocket.reset();
}

void NetworkPlugin::setNetworkErrorCallback(std::function<void(const std::string&)> callback) { errorCallback = std::move(callback); }
void NetworkPlugin::setReconnectCallback(std::function<void()> callback) { reconnectCallback = std::move(callback); }

void NetworkPlugin::reconnectToServer(const Endpoint& serverEndpoint, Protocol protocol) {
    disconnectFromServer();
    connectToServer(serverEndpoint, protocol);
}

void NetworkPlugin::sendPacket(const Packet& packet, Protocol protocol, const Endpoint* endpoint) {
    auto buffer = serializePacket(packet);
    auto& sock = (protocol == Protocol::TCP) ? tcpSocket : udpSocket;
    if (sock && sock->isConnected()) {
        sock->send(buffer, endpoint);
    }
}

Packet NetworkPlugin::receivePacket(Protocol protocol, Endpoint* endpoint) {
    std::vector<uint8_t> buffer;
    auto& sock = (protocol == Protocol::TCP) ? tcpSocket : udpSocket;
    if (sock && sock->isConnected()) {
        sock->recv(buffer, endpoint);
    }
    return buffer.empty() ? Packet{} : deserializePacket(buffer);
}

uint64_t NetworkPlugin::now_ns() {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count());
}

void NetworkPlugin::resolveServerConflict(const std::vector<uint8_t>&, std::vector<uint8_t>&) {}

void NetworkPlugin::sendRawTcp(const std::vector<uint8_t> &buffer, const Endpoint &endpoint) {
    if (tcpSocket && tcpSocket->isConnected()) {
        tcpSocket->send(buffer, &endpoint);
    }
}

void NetworkPlugin::recvRawTcp(std::vector<uint8_t> &buffer, Endpoint *endpoint) {
    if (tcpSocket && tcpSocket->isConnected()) {
        tcpSocket->recv(buffer, endpoint);
    }
}

std::vector<uint8_t> serializePacket(const Packet& packet) {
    std::vector<uint8_t> buffer;
    buffer.resize(21);
    size_t offset = 0;
    auto write16 = [&](uint16_t v) { v = htons(v); memcpy(&buffer[offset], &v, 2); offset += 2; };
    auto write32 = [&](uint32_t v) { v = htonl(v); memcpy(&buffer[offset], &v, 4); offset += 4; };
    auto write8 = [&](uint8_t v) { buffer[offset++] = v; };
    write16(packet.magic); write8(packet.version); write8(packet.flags);
    write32(packet.sequence); write32(packet.ackBase);
    write8(packet.ackBits); write8(packet.channel);
    write16(packet.size); write32(packet.clientId); write8(packet.command);
    buffer.insert(buffer.end(), packet.payload.begin(), packet.payload.end());
    return buffer;
}

Packet deserializePacket(const std::vector<uint8_t>& buffer) {
    Packet packet;
    if (buffer.size() < 21) return packet;
    size_t offset = 0;
    auto read16 = [&]() { uint16_t v; memcpy(&v, &buffer[offset], 2); offset += 2; return ntohs(v); };
    auto read32 = [&]() { uint32_t v; memcpy(&v, &buffer[offset], 4); offset += 4; return ntohl(v); };
    auto read8 = [&]() { return buffer[offset++]; };
    packet.magic = read16(); packet.version = read8(); packet.flags = read8();
    packet.sequence = read32(); packet.ackBase = read32();
    packet.ackBits = read8(); packet.channel = read8();
    packet.size = read16(); packet.clientId = read32(); packet.command = read8();
    if (buffer.size() > offset)
        packet.payload.assign(buffer.begin() + static_cast<ptrdiff_t>(offset), buffer.end());
    return packet;
}

} // namespace r::net
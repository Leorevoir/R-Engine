#include "../../include/R-Engine/Plugins/NetworkPlugin.hpp"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>

using namespace r::net;

int main() {
    NetworkPlugin plugin;
    Endpoint tcpEndpoint{"127.0.0.1", 3000};

    try {
        plugin.connectToServer(tcpEndpoint, Protocol::TCP);
        std::cout << "TCP connection successful.\n";

        struct JoinPacket {
            uint16_t magic;
            uint8_t version;
            uint8_t flags;
            uint8_t cmd;
            uint32_t gameId;
        };

        JoinPacket join;
        join.magic = htons(0x4257);
        join.version = 1;
        join.flags = 0;
        join.cmd = 1;
        join.gameId = htonl(0x12345678);

        std::vector<uint8_t> buffer(sizeof(JoinPacket));
        memcpy(buffer.data(), &join, sizeof(JoinPacket));

        plugin.sendRawTcp(buffer, tcpEndpoint);
        std::cout << "Sent minimal JOIN packet\n";

        std::vector<uint8_t> recvBuffer;
        plugin.recvRawTcp(recvBuffer, nullptr);
        std::cout << "Received raw response: ";
        for (auto c : recvBuffer) printf("%02x ", c);
        std::cout << std::endl;

        plugin.disconnectFromServer();
        std::cout << "TCP disconnection successful.\n";
    } catch (const std::exception& e) {
        std::cerr << "TCP test error: " << e.what() << std::endl;
    }

    return 0;
}
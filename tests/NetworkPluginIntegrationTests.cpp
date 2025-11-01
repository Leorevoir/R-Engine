#include <gtest/gtest.h>
#include "../../src/Plugins/Network/NetworkPlugin.hpp"
#include <thread>
#include <chrono>

using namespace rtype::network;

class MockServer {
public:
    MockServer() : running(false) {}

    void start(uint16_t port) {
        running = true;
        serverThread = std::thread([this, port]() {
            while (running) {
                // Simulate server behavior (e.g., accept connections, send/receive data)
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }

    void stop() {
        running = false;
        if (serverThread.joinable()) {
            serverThread.join();
        }
    }

private:
    bool running;
    std::thread serverThread;
};

TEST(NetworkPluginIntegrationTest, ConnectToServer) {
    MockServer server;
    server.start(8080);

    NetworkPlugin plugin;
    Endpoint serverEndpoint{"127.0.0.1", 8080};

    EXPECT_NO_THROW(plugin.connectToServer(serverEndpoint));
    plugin.disconnectFromServer();

    server.stop();
}

TEST(NetworkPluginIntegrationTest, SendAndReceiveMessages) {
    MockServer server;
    server.start(8080);

    NetworkPlugin plugin;
    Endpoint serverEndpoint{"127.0.0.1", 8080};
    plugin.connectToServer(serverEndpoint);

    json message = { {"type", "test"}, {"data", {"key", "value"}} };
    EXPECT_NO_THROW(plugin.sendMessage(message));

    auto receivedMessage = plugin.receiveMessage();
    EXPECT_EQ(receivedMessage["type"], "test");
    EXPECT_EQ(receivedMessage["data"]["key"], "value");

    plugin.disconnectFromServer();
    server.stop();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
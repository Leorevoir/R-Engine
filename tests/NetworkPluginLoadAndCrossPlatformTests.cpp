#include <gtest/gtest.h>
#include "../../src/Plugins/Network/NetworkPlugin.hpp"
#include <thread>
#include <vector>
#include <chrono>

using namespace rtype::network;

class MockLoadServer {
public:
    MockLoadServer() : running(false) {}

    void start(uint16_t port) {
        running = true;
        serverThread = std::thread([this, port]() {
            while (running) {
                // Simulate server load handling
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

TEST(NetworkPluginLoadTest, HighLoadHandling) {
    MockLoadServer server;
    server.start(8080);

    NetworkPlugin plugin;
    Endpoint serverEndpoint{"127.0.0.1", 8080};
    plugin.connectToServer(serverEndpoint);

    const int messageCount = 1000;
    json message = { {"type", "test"}, {"data", {"key", "value"}} };

    for (int i = 0; i < messageCount; ++i) {
        EXPECT_NO_THROW(plugin.sendMessage(message));
    }

    plugin.disconnectFromServer();
    server.stop();
}

TEST(NetworkPluginCrossPlatformTest, BasicFunctionality) {
    // This test assumes the plugin is compiled and run on multiple platforms.
    NetworkPlugin plugin;
    Endpoint serverEndpoint{"127.0.0.1", 8080};

    EXPECT_NO_THROW(plugin.connectToServer(serverEndpoint));
    EXPECT_NO_THROW(plugin.disconnectFromServer());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
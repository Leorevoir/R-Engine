#include "../../src/Plugins/Network/NetworkPlugin.hpp"
#include <gtest/gtest.h>

using namespace rtype::network;

TEST(NetworkPluginTests, ServerConflictResolution) {
    NetworkPlugin plugin;

    std::vector<uint8_t> serverData = {1, 2, 3, 4};
    std::vector<uint8_t> clientData = {5, 6, 7, 8};

    plugin.resolveServerConflict(serverData, clientData);

    ASSERT_EQ(clientData, serverData);
}

TEST(NetworkPluginTests, TimeSynchronization) {
    NetworkPlugin plugin;

    uint64_t time1 = plugin.now_ns();
    uint64_t time2 = plugin.now_ns();

    ASSERT_LE(time1, time2);
}

TEST(NetworkPluginTests, MessageSerialization) {
    Message message(42, 123456789, {10, 20, 30});

    NetworkPlugin plugin;
    plugin.connectToServer({"127.0.0.1", 8080});

    plugin.sendMessage(message);
    Message receivedMessage = plugin.receiveMessage();

    ASSERT_EQ(message.id, receivedMessage.id);
    ASSERT_EQ(message.timestamp, receivedMessage.timestamp);
    ASSERT_EQ(message.payload, receivedMessage.payload);
}

TEST(SocketTest, ConnectDisconnect) {
    Endpoint endpoint{"127.0.0.1", 8080};
    Socket socket(Protocol::TCP);

    EXPECT_NO_THROW(socket.connect(endpoint));
    EXPECT_NO_THROW(socket.disconnect());
}

TEST(SocketTest, SendReceive) {
    Socket socket(Protocol::TCP);
    std::vector<uint8_t> dataToSend = {1, 2, 3, 4};
    std::vector<uint8_t> buffer;

    EXPECT_NO_THROW(socket.send(dataToSend));
    EXPECT_NO_THROW(socket.recv(buffer));
    EXPECT_EQ(buffer.size(), 1024); // Example buffer size
}

TEST(SerializationTest, SerializeDeserialize) {
    json message = { {"type", "test"}, {"data", {"key", "value"}} };
    auto serialized = serializeMessage(message);
    auto deserialized = deserializeMessage(serialized);

    EXPECT_EQ(message, deserialized);
}

TEST(EncryptionTest, EncryptDecrypt) {
    std::vector<uint8_t> data = {1, 2, 3, 4};
    std::string key = "default_key_32_bytes_long";

    auto encrypted = encryptData(data, key);
    auto decrypted = decryptData(encrypted, key);

    EXPECT_EQ(data, decrypted);
}
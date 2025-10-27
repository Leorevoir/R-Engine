#include "NetworkPlugin.hpp"
#include <catch2/catch_test_macros.hpp>
#include <thread>

using namespace rtype::network;

TEST_CASE("TCP Connection: valid address/port", "[network][tcp]") {
    NetworkPlugin plugin;
    Endpoint serverEndpoint{"127.0.0.1", 4000};
    REQUIRE_NOTHROW(plugin.connectToServer(serverEndpoint, Protocol::TCP));
    plugin.disconnectFromServer();
}

TEST_CASE("TCP Connection: invalid address", "[network][tcp]") {
    NetworkPlugin plugin;
    Endpoint serverEndpoint{"256.256.256.256", 4000};
    REQUIRE_THROWS(plugin.connectToServer(serverEndpoint, Protocol::TCP));
}

TEST_CASE("TCP Connection: invalid port", "[network][tcp]") {
    NetworkPlugin plugin;
    Endpoint serverEndpoint{"127.0.0.1", 99999};
    REQUIRE_THROWS(plugin.connectToServer(serverEndpoint, Protocol::TCP));
}

TEST_CASE("UDP Connection: valid address/port", "[network][udp]") {
    NetworkPlugin plugin;
    Endpoint serverEndpoint{"127.0.0.1", 4001};
    REQUIRE_NOTHROW(plugin.connectToServer(serverEndpoint, Protocol::UDP));
    plugin.disconnectFromServer();
}

TEST_CASE("UDP Connection: invalid address", "[network][udp]") {
    NetworkPlugin plugin;
    Endpoint serverEndpoint{"invalid_address", 4001};
    REQUIRE_THROWS(plugin.connectToServer(serverEndpoint, Protocol::UDP));
}

TEST_CASE("UDP Connection: invalid port", "[network][udp]") {
    NetworkPlugin plugin;
    Endpoint serverEndpoint{"127.0.0.1", 99999};
    REQUIRE_THROWS(plugin.connectToServer(serverEndpoint, Protocol::UDP));
}

TEST_CASE("Proper disconnection and automatic reconnection", "[network][reconnect]") {
    NetworkPlugin plugin;
    Endpoint serverEndpoint{"127.0.0.1", 4000};
    plugin.connectToServer(serverEndpoint, Protocol::TCP);
    plugin.disconnectFromServer();
    REQUIRE_NOTHROW(plugin.reconnectToServer(serverEndpoint, Protocol::TCP));
    plugin.disconnectFromServer();
}

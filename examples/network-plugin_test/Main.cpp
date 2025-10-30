#include "R-Engine/Application.hpp"
#include "R-Engine/Core/Logger.hpp"
#include "R-Engine/ECS/RunConditions.hpp"
#include "R-Engine/Plugins/DefaultPlugins.hpp"
#include "R-Engine/Plugins/InputPlugin.hpp"
#include "R-Engine/Plugins/NetworkPlugin.hpp"
#include <R-Engine/Core/Backend.hpp>

#include <iostream>

/**
 * @brief Defines the application's state.
 *
 * This simple state machine helps manage the network connection flow.
 * The application will start in the `Connecting` state and move to `Connected`
 * or `Error` based on network events.
 */
enum class AppState {
    Connecting,
    Connected,
    Error,
};

/**
 * @brief System that runs once at startup to initiate the network connection.
 *
 * It creates a NetworkConnectEvent and sends it, which will be processed by the
 * NetworkPlugin's systems to establish a connection.
 */
static void connect_to_server_system(r::ecs::EventWriter<r::net::NetworkConnectEvent> connect_writer)
{
    r::Logger::info("Attempting to connect to server...");
    connect_writer.send(r::net::NetworkConnectEvent{
        .endpoint = {"127.0.0.1", 4242},///< Target server endpoint
        .protocol = r::net::Protocol::TCP,
    });
}

/**
 * @brief System that monitors the network connection status and updates the AppState.
 */
static void check_connection_status_system(r::ecs::Res<r::net::Connection> conn, r::ecs::ResMut<r::NextState<AppState>> next_state)
{
    if (conn.ptr->connected) {
        next_state.ptr->set(AppState::Connected);
    }
}

/**
 * @brief System that listens for keyboard input to send a packet.
 *
 * This system only runs when the application is in the `Connected` state.
 * When the SPACE key is pressed, it constructs and sends a NetworkSendEvent.
 */
static void send_packet_on_input_system(r::ecs::Res<r::UserInput> user_input, r::ecs::EventWriter<r::net::NetworkSendEvent> send_writer)
{
    if (user_input.ptr->isKeyPressed(KEY_SPACE)) {
        r::Logger::info("Spacebar pressed! Sending a packet...");

        /* Create a sample packet to send */
        r::net::Packet packet_to_send = {
            .magic = 0x4257,
            .version = 1,
            .flags = 0,
            .sequence = 1,
            .ackBase = 0,
            .ackBits = 0,
            .channel = 0,
            .size = 0,///< Size is set automatically during serialization
            .clientId = 123,
            .command = 1,///< Command for "Join Game"
        };

        /* Create a payload */
        std::string message = "Hello, Server!";
        packet_to_send.payload.assign(message.begin(), message.end());

        /* Send the event. The NetworkPlugin's systems will handle the rest. */
        send_writer.send({packet_to_send});
    }
}

/**
 * @brief System to process and log incoming network messages.
 *
 * It iterates through all NetworkMessageEvents received during the frame and
 * prints their content to the console.
 */
static void receive_message_system(r::ecs::EventReader<r::net::NetworkMessageEvent> message_reader)
{
    if (!message_reader.has_events()) {
        return;
    }

    for (const auto &message : message_reader) {
        r::Logger::info("Received a message from server!");
        std::cout << "  - Message Type: " << static_cast<int>(message.message_type) << std::endl;

        /* Print payload as a string for demo */
        std::string payload_str(message.payload.begin(), message.payload.end());
        std::cout << "  - Payload: \"" << payload_str << "\"" << std::endl;
    }
}

/**
 * @brief System to log any network errors.
 *
 * This system listens for NetworkErrorEvents and prints the error message,
 * then transitions the application to an `Error` state.
 */
static void log_network_errors_system(r::ecs::EventReader<r::net::NetworkErrorEvent> error_reader,
    r::ecs::ResMut<r::NextState<AppState>> next_state)
{
    if (!error_reader.has_events()) {
        return;
    }

    for (const auto &error : error_reader) {
        r::Logger::error("Network Error: " + error.message);
    }

    next_state.ptr->set(AppState::Error);
}

int main()
{
    r::Application{}
        .add_plugins(r::DefaultPlugins(), r::net::NetworkPlugin())

        /* Initialize the state machine. */
        .init_state(AppState::Connecting)

        /* Add our custom systems to the application's schedule. */
        .add_systems<connect_to_server_system>(r::Schedule::STARTUP)

        /* These systems will run every frame in the UPDATE schedule. */
        .add_systems<log_network_errors_system>(r::Schedule::UPDATE)
        .add_systems<receive_message_system>(r::Schedule::UPDATE)

        /* Use run conditions to control when systems are active. */
        .add_systems<check_connection_status_system>(r::Schedule::UPDATE)
        .run_if<r::run_conditions::in_state<AppState::Connecting>>()

        .add_systems<send_packet_on_input_system>(r::Schedule::UPDATE)
        .run_if<r::run_conditions::in_state<AppState::Connected>>()

        /* Start the engine's main loop. */
        .run();

    return 0;
}

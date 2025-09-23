#include "raylib.h"
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Core/Backend.hpp>

bool r::UserInput::isKeyPressed(int raylib_key_code) const
{
    return keys_pressed.count(raylib_key_code);
}

bool r::UserInput::isMouseButtonPressed(int raylib_button_code) const
{
    return mouse_buttons_pressed.count(raylib_button_code);
}

/**
 * @brief Bind an action (action name) to a specific input. Inputs can be keyboard or mouse types.
 * @details This system adds a name to a specific key
 */
void r::InputMap::bindAction(const std::string& action_name, InputType type, int raylib_key_code)
{
    action_to_keys[action_name].push_back({type, raylib_key_code});
}

/**
 * @brief Check if an input has been triggered by his name
 *
 */
bool r::InputMap::isActionPressed(const std::string& action_name, const r::UserInput& userInput) const
{
    if (action_to_keys.count(action_name)) {
        const std::vector<Binding>& keys = action_to_keys.at(action_name);

        for (Binding binding : keys) {
            if (binding.type == KEYBOARD) {
                if (userInput.isKeyPressed(binding.code))
                    return true;
            } else {
                if (userInput.isMouseButtonPressed(binding.code))
                    return true;
            }
        }
    }
    return false;
}

static void input_system(r::ecs::Res<r::UserInput> userInput)
{
    auto* mutable_state = const_cast<r::UserInput*>(userInput.ptr);

    mutable_state->keys_pressed.clear();
    for (int key = 32; key < 348; ++key) {
        if (IsKeyDown(key)) {
            mutable_state->keys_pressed.insert(key);
        }
    }

    mutable_state->mouse_buttons_pressed.clear();
    for (int button = 0; button < 3; ++button) {
        if (IsMouseButtonPressed(button)) {
            mutable_state->mouse_buttons_pressed.insert(button);
        }
    }
}

void r::InputPlugin::build(Application& app)
{
    app
        .insert_resource(UserInput{})
        .insert_resource(InputMap{})
        .add_systems(Schedule::UPDATE, input_system);

    Logger::info("InputPlugin built");
}

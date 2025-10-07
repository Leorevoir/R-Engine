#include "R-Engine/ECS/Query.hpp"
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Core/Backend.hpp>

static constexpr u16 FIRST_KEY = 32;
static constexpr u16 MAX_KEY = 348;

bool r::UserInput::isKeyDown(int key_code) const
{
    const auto it = keys_down.find(key_code);
    return it != keys_down.end();
}

bool r::UserInput::isKeyPressed(int key_code) const
{
    const auto it = keys_pressed.find(key_code);

    return it != keys_pressed.end();
}

bool r::UserInput::isKeyReleased(int key_code) const
{
    const auto it = keys_released.find(key_code);
    return it != keys_released.end();
}

bool r::UserInput::isMouseButtonPressed(int button_code) const
{
    const auto it = mouse_buttons_pressed.find(button_code);

    return it != mouse_buttons_pressed.end();
}

bool r::UserInput::isMouseButtonDown(int button_code) const
{
    const auto it = mouse_buttons_down.find(button_code);

    return it != mouse_buttons_down.end();
}

bool r::UserInput::isMouseButtonReleased(int button_code) const
{
    const auto it = mouse_buttons_released.find(button_code);

    return it != mouse_buttons_released.end();
}
/**
 * @brief Bind an action (action name) to a specific input. Inputs can be keyboard or mouse types.
 * @details This system adds a name to a specific key
 */
void r::InputMap::bindAction(const std::string& action_name, InputType type, u16 key_code)
{
    action_to_keys[action_name].push_back({type, key_code});
}

/**
 * @brief Check if an input has been triggered by his name
 *  
 */
bool r::InputMap::isActionPressed(const std::string& action_name, const r::UserInput& userInput) const
{
    const auto it = action_to_keys.find(action_name);

    if (it == action_to_keys.end())
        return false;
    const std::vector<Binding> &keys = it->second;

    for (const Binding &binding : keys) {
        if (binding.type == KEYBOARD && userInput.isKeyPressed(binding.code))
            return true;
        if (binding.type == MOUSE && userInput.isMouseButtonPressed(binding.code))
            return true;
    }
    return false;
}

static void input_system(r::ecs::ResMut<r::UserInput> userInput)
{
    userInput.ptr->keys_down.clear();
    userInput.ptr->keys_pressed.clear();
    userInput.ptr->keys_released.clear();
    for (u16 key = FIRST_KEY; key < MAX_KEY; ++key) {
        if (IsKeyDown(key)) {
            userInput.ptr->keys_down.insert(key);
        }
        if (IsKeyPressed(key)) {
            userInput.ptr->keys_pressed.insert(key);
        }
        if (IsKeyReleased(key)) {
            userInput.ptr->keys_released.insert(key);
        }
    }

    /* 3 because we count 3 buttons on the mouse */
    userInput.ptr->mouse_buttons_pressed.clear();
    userInput.ptr->mouse_buttons_down.clear();
    userInput.ptr->mouse_buttons_released.clear();
    for (u16 button = 0; button < 3; ++button) {
        if (IsMouseButtonPressed(button)) {
            userInput.ptr->mouse_buttons_pressed.insert(button);
        }
        if (IsMouseButtonDown(button)) {
            userInput.ptr->mouse_buttons_down.insert(button);
        }
        if (IsMouseButtonReleased(button)) {
            userInput.ptr->mouse_buttons_released.insert(button);
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

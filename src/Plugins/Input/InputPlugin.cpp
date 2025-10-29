#include <R-Engine/Plugins/InputPlugin.hpp>

#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/ECS/Query.hpp>

#ifndef to_vec2f
    #define to_vec2f(v)                                                                                                                    \
        r::Vec2f                                                                                                                           \
        {                                                                                                                                  \
            v.x, v.y                                                                                                                       \
        }
#endif

static constexpr u16 FIRST_KEY = 32;
static constexpr u16 MAX_KEY = 348;

bool r::UserInput::isKeyPressed(i32 key_code) const
{
    const auto it = keys_pressed.find(key_code);

    return it != keys_pressed.end();
}

bool r::UserInput::isMouseButtonPressed(i32 button_code) const
{
    const auto it = mouse_buttons_pressed.find(button_code);

    return it != mouse_buttons_pressed.end();
}

bool r::UserInput::isGamepadButtonPressed(i32 button_code) const
{
    const auto it = gamepad_buttons_pressed.find(button_code);
    return it != gamepad_buttons_pressed.end();
}

r::Vec2f r::UserInput::getGamepadAxis(i32 gamepad_id) const
{
    const auto it = gamepad_axis_values.find(gamepad_id);
    if (it != gamepad_axis_values.end()) {
        return it->second;
    }
    return {0.0f, 0.0f};
}

/**
 * @brief Bind an action (action name) to a specific input. Inputs can be keyboard or mouse types.
 * @details This system adds a name to a specific key
 */
void r::InputMap::bindAction(const std::string &action_name, InputType type, u16 key_code)
{
    action_to_keys[action_name].push_back({type, key_code});
}

/**
 * @brief Check if an input has been triggered by his name
 *
 */
bool r::InputMap::isActionPressed(const std::string &action_name, const r::UserInput &userInput) const
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
        if (binding.type == GAMEPAD && userInput.isGamepadButtonPressed(binding.code))
            return true;
    }
    return false;
}

static void input_system(r::ecs::ResMut<r::UserInput> userInput)
{
    userInput.ptr->keys_pressed.clear();
    for (u16 key = FIRST_KEY; key < MAX_KEY; ++key) {
        if (IsKeyDown(key)) {
            userInput.ptr->keys_pressed.insert(key);
        }
    }

    /* 3 Because we count 3 buttons on the mouse */
    userInput.ptr->mouse_buttons_pressed.clear();
    for (u16 button = 0; button < 3; ++button) {
        if (IsMouseButtonDown(button)) {
            userInput.ptr->mouse_buttons_pressed.insert(button);
        }
    }

    userInput.ptr->gamepad_buttons_pressed.clear();
    userInput.ptr->gamepad_axis_values.clear();

    if (IsGamepadAvailable(0)) {
        /* Poll all gamepad buttons up to a reasonable limit */
        for (int button = 0; button < 20; ++button) {
            if (IsGamepadButtonDown(0, button)) {
                userInput.ptr->gamepad_buttons_pressed.insert(button);
            }
        }
        /* Poll left analog stick */
        userInput.ptr->gamepad_axis_values[0] = {GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X),
            GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y)};
    }

    userInput.ptr->mouse_position = to_vec2f(GetMousePosition());
    userInput.ptr->mouse_delta = to_vec2f(GetMouseDelta());
}

void r::InputPlugin::build(Application &app)
{
    app.insert_resource(UserInput{}).insert_resource(InputMap{}).add_systems<input_system>(Schedule::UPDATE);

    Logger::debug("InputPlugin built");
}

#undef to_vec2f

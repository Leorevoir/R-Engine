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

/* --- UserInput Method Implementations --- */

bool r::UserInput::isKeyPressed(i32 key_code) const
{
    return keys_pressed.count(key_code) > 0;
}

bool r::UserInput::isKeyReleased(i32 key_code) const
{
    /* Was pressed last frame, but not this frame */
    return prev_keys_pressed.count(key_code) > 0 && keys_pressed.count(key_code) == 0;
}

bool r::UserInput::isKeyJustPressed(i32 key_code) const
{
    /* Is pressed this frame, but wasn't last frame */
    return keys_pressed.count(key_code) > 0 && prev_keys_pressed.count(key_code) == 0;
}

bool r::UserInput::isMouseButtonPressed(i32 button_code) const
{
    return mouse_buttons_pressed.count(button_code) > 0;
}

bool r::UserInput::isMouseButtonReleased(i32 button_code) const
{
    return prev_mouse_buttons_pressed.count(button_code) > 0 && mouse_buttons_pressed.count(button_code) == 0;
}

bool r::UserInput::isMouseButtonJustPressed(i32 button_code) const
{
    return mouse_buttons_pressed.count(button_code) > 0 && prev_mouse_buttons_pressed.count(button_code) == 0;
}

bool r::UserInput::isGamepadButtonPressed(i32 button_code) const
{
    return gamepad_buttons_pressed.count(button_code) > 0;
}

bool r::UserInput::isGamepadButtonReleased(i32 button_code) const
{
    return prev_gamepad_buttons_pressed.count(button_code) > 0 && gamepad_buttons_pressed.count(button_code) == 0;
}

bool r::UserInput::isGamepadButtonJustPressed(i32 button_code) const
{
    return gamepad_buttons_pressed.count(button_code) > 0 && prev_gamepad_buttons_pressed.count(button_code) == 0;
}

r::Vec2f r::UserInput::getGamepadAxis(i32 gamepad_id) const
{
    const auto it = gamepad_axis_values.find(gamepad_id);
    if (it != gamepad_axis_values.end()) {
        return it->second;
    }
    return {0.0f, 0.0f};
}

/* --- InputMap Method Implementations --- */

void r::InputMap::bindAction(const std::string &action_name, InputType type, u16 key_code)
{
    action_to_keys[action_name].push_back({type, key_code});
}

bool r::InputMap::isActionPressed(const std::string &action_name, const r::UserInput &userInput) const
{
    const auto it = action_to_keys.find(action_name);

    if (it == action_to_keys.end())
        return false;

    for (const auto &binding : it->second) {
        if (binding.type == KEYBOARD && userInput.isKeyPressed(binding.code))
            return true;
        if (binding.type == MOUSE && userInput.isMouseButtonPressed(binding.code))
            return true;
        if (binding.type == GAMEPAD && userInput.isGamepadButtonPressed(binding.code))
            return true;
    }
    return false;
}

bool r::InputMap::isActionReleased(const std::string &action_name, const r::UserInput &userInput) const
{
    const auto it = action_to_keys.find(action_name);
    if (it == action_to_keys.end())
        return false;

    for (const auto &binding : it->second) {
        if (binding.type == KEYBOARD && userInput.isKeyReleased(binding.code))
            return true;
        if (binding.type == MOUSE && userInput.isMouseButtonReleased(binding.code))
            return true;
        if (binding.type == GAMEPAD && userInput.isGamepadButtonReleased(binding.code))
            return true;
    }
    return false;
}

bool r::InputMap::isActionJustPressed(const std::string &action_name, const r::UserInput &userInput) const
{
    const auto it = action_to_keys.find(action_name);
    if (it == action_to_keys.end())
        return false;

    for (const auto &binding : it->second) {
        if (binding.type == KEYBOARD && userInput.isKeyJustPressed(binding.code))
            return true;
        if (binding.type == MOUSE && userInput.isMouseButtonJustPressed(binding.code))
            return true;
        if (binding.type == GAMEPAD && userInput.isGamepadButtonJustPressed(binding.code))
            return true;
    }
    return false;
}

/* --- The core input system --- */

static void input_system(r::ecs::ResMut<r::UserInput> userInput)
{
    /* 1. Copy last frame's state into the "previous" sets */
    userInput.ptr->prev_keys_pressed = userInput.ptr->keys_pressed;
    userInput.ptr->prev_mouse_buttons_pressed = userInput.ptr->mouse_buttons_pressed;
    userInput.ptr->prev_gamepad_buttons_pressed = userInput.ptr->gamepad_buttons_pressed;

    /* 2. Clear the current state sets to repopulate them */
    userInput.ptr->keys_pressed.clear();
    userInput.ptr->mouse_buttons_pressed.clear();
    userInput.ptr->gamepad_buttons_pressed.clear();

    /* 3. Poll hardware and fill the current state for this frame */
    for (u16 key = FIRST_KEY; key < MAX_KEY; ++key) {
        if (IsKeyDown(key)) {
            userInput.ptr->keys_pressed.insert(key);
        }
    }

    for (u16 button = 0; button < 3; ++button) {
        if (IsMouseButtonDown(button)) {
            userInput.ptr->mouse_buttons_pressed.insert(button);
        }
    }

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

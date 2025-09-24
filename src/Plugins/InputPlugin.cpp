#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Core/Backend.hpp>

static constexpr u16 FIRST_KEY = 32;
static constexpr u16 MAX_KEY = 348;

bool r::UserInput::isKeyPressed(int key_code) const
{
    const auto it = keys_pressed.find(key_code);

    return it != keys_pressed.end();
}

bool r::UserInput::isMouseButtonPressed(int button_code) const
{
    const auto it = mouse_buttons_pressed.find(button_code);

    return it != mouse_buttons_pressed.end();
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

static void input_system(r::ecs::Res<r::UserInput> userInput)
{
    auto* mutable_state = const_cast<r::UserInput*>(userInput.ptr);

    mutable_state->keys_pressed.clear();
    for (u16 key = FIRST_KEY; key < MAX_KEY; ++key) {
        if (IsKeyDown(key)) {
            mutable_state->keys_pressed.insert(key);
        }
    }

    /* 3 Because we count 3 buttons on the mouse */
    mutable_state->mouse_buttons_pressed.clear();
    for (u16 button = 0; button < 3; ++button) {
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

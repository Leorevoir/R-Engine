#pragma once

#include <R-Engine/Maths/Vec.hpp>
#include <R-Engine/Plugins/Plugin.hpp>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace r {

/**
 * @brief Identifies the type of input device.
 */
enum InputType { KEYBOARD, MOUSE, GAMEPAD };

/**
 * @brief Associates an input type with a specific key/button code.
 */
struct R_ENGINE_API Binding {
        InputType type;
        u16 code;
};

/**
 * @brief A resource that holds the current and previous frame's input state.
 * @details Updated automatically each frame to provide a consistent snapshot for all systems.
 */
struct R_ENGINE_API UserInput {

        /* Current frame state */
        std::unordered_set<i32> keys_pressed;
        std::unordered_set<i32> mouse_buttons_pressed;
        std::unordered_set<i32> gamepad_buttons_pressed;

        /* Previous frame state */
        std::unordered_set<i32> prev_keys_pressed;
        std::unordered_set<i32> prev_mouse_buttons_pressed;
        std::unordered_set<i32> prev_gamepad_buttons_pressed;

        Vec2f mouse_position = {.0f, .0f};
        Vec2f mouse_delta = {.0f, .0f};
        std::unordered_map<i32, Vec2f> gamepad_axis_values;

        /**
         * @brief Checks if a key is currently held down.
         */
        bool isKeyPressed(i32 key_code) const;
        /**
         * @brief Checks if a key was released this frame.
         */
        bool isKeyReleased(i32 key_code) const;
        /**
         * @brief Checks if a key was initially pressed this frame.
         */
        bool isKeyJustPressed(i32 key_code) const;

        /**
         * @brief Checks if a mouse button is currently held down.
         */
        bool isMouseButtonPressed(i32 button_code) const;
        /**
         * @brief Checks if a mouse button was released this frame.
         */
        bool isMouseButtonReleased(i32 button_code) const;
        /**
         * @brief Checks if a mouse button was initially pressed this frame.
         */
        bool isMouseButtonJustPressed(i32 button_code) const;

        /**
         * @brief Checks if a gamepad button is currently held down.
         */
        bool isGamepadButtonPressed(i32 button_code) const;
        /**
         * @brief Checks if a gamepad button was released this frame.
         */
        bool isGamepadButtonReleased(i32 button_code) const;
        /**
         * @brief Checks if a gamepad button was initially pressed this frame.
         */
        bool isGamepadButtonJustPressed(i32 button_code) const;

        /**
         * @brief Gets the current value of a gamepad's analog stick.
         */
        Vec2f getGamepadAxis(i32 gamepad_id) const;
};

/**
 * @brief A resource for mapping abstract actions (e.g., "jump") to physical inputs.
 */
struct R_ENGINE_API InputMap {
        std::unordered_map<std::string, std::vector<Binding>> action_to_keys;

        /**
         * @brief Binds a physical key/button to a named action.
         */
        void bindAction(const std::string &action_name, InputType type, u16 key_code);

        /**
         * @brief Checks if any binding for an action is currently held down.
         */
        bool isActionPressed(const std::string &action_name, const UserInput &userInput) const;
        /**
         * @brief Checks if any binding for an action was released this frame.
         */
        bool isActionReleased(const std::string &action_name, const UserInput &userInput) const;
        /**
         * @brief Checks if any binding for an action was initially pressed this frame.
         */
        bool isActionJustPressed(const std::string &action_name, const UserInput &userInput) const;
};

/**
 * @brief Adds input handling resources and systems to the application.
 */
class R_ENGINE_API InputPlugin final : public Plugin
{
    public:
        /**
         * @brief Configures the application to handle user input.
         * @param app The application to configure.
         */
        void build(Application &app) override;
};

}// namespace r

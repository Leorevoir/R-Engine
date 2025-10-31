#pragma once

#include <R-Engine/Maths/Vec.hpp>
#include <R-Engine/Plugins/Plugin.hpp>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace r {

enum InputType { KEYBOARD, MOUSE, GAMEPAD };

struct R_ENGINE_API Binding {
        InputType type;
        u16 code;
};

struct R_ENGINE_API UserInput {

        std::unordered_set<i32> keys_pressed;
        std::unordered_set<i32> mouse_buttons_pressed;
        std::unordered_set<i32> gamepad_buttons_pressed;

        Vec2f mouse_position = {.0f, .0f};
        Vec2f mouse_delta = {.0f, .0f};
        std::unordered_map<i32, Vec2f> gamepad_axis_values;

        bool isKeyPressed(i32 key_code) const;
        bool isMouseButtonPressed(i32 button_code) const;
        bool isGamepadButtonPressed(i32 button_code) const;
        Vec2f getGamepadAxis(i32 gamepad_id) const;
};

struct R_ENGINE_API InputMap {
        std::unordered_map<std::string, std::vector<Binding>> action_to_keys;

        void bindAction(const std::string &action_name, InputType type, u16 key_code);

        bool isActionPressed(const std::string &action_name, const UserInput &userInput) const;
};

class R_ENGINE_API InputPlugin final : public Plugin
{
    public:
        void build(Application &app) override;
};

}// namespace r

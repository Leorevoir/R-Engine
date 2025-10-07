#pragma once

#include "R-Engine/Types.hpp"
#include <R-Engine/Plugins/Plugin.hpp>
#include <unordered_set>
#include <unordered_map>
#include <string>   
#include <vector>

namespace r {

enum InputType
{
    KEYBOARD,
    MOUSE
};

struct R_ENGINE_API Binding {
    InputType type;
    u16 code;
};

struct R_ENGINE_API UserInput {

    std::unordered_set<int> keys_down;            /* keyboard state (held) */
    std::unordered_set<int> keys_pressed;         /* keyboard edge pressed */
    std::unordered_set<int> keys_released;        /* keyboard edge released */
    std::unordered_set<int> mouse_buttons_pressed;
    std::unordered_set<int> mouse_buttons_down;
    std::unordered_set<int> mouse_buttons_released;

    bool isKeyDown(int key_code) const;
    bool isKeyPressed(int key_code) const;
    bool isKeyReleased(int key_code) const;
    bool isMouseButtonPressed(int button_code) const;
    bool isMouseButtonDown(int button_code) const;
    bool isMouseButtonReleased(int button_code) const;
};

struct R_ENGINE_API InputMap {
    std::unordered_map<std::string, std::vector<Binding>> action_to_keys;

    void bindAction(const std::string& action_name, InputType type, u16 key_code);

    bool isActionPressed(const std::string& action_name, const UserInput& userInput) const;
};

class R_ENGINE_API InputPlugin final : public Plugin
{
    public:
        void build(Application &app) override;
};

}

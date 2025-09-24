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

struct Binding
{
    InputType type;
    u16 code;
};

struct UserInput {

    std::unordered_set<int> keys_pressed;
    std::unordered_set<int> mouse_buttons_pressed;

    bool isKeyPressed(int key_code) const;
    bool isMouseButtonPressed(int button_code) const;
};

struct InputMap {
    std::unordered_map<std::string, std::vector<Binding>> action_to_keys;

    void bindAction(const std::string& action_name, InputType type, u16 key_code);

    bool isActionPressed(const std::string& action_name, const UserInput& userInput) const;
};

class InputPlugin final : public Plugin {

public:
    void build(Application &app) override;
};

}
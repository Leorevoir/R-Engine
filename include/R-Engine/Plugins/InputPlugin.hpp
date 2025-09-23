#pragma once

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
    int code;
};

struct UserInput {

    std::unordered_set<int> keys_pressed;
    std::unordered_set<int> mouse_buttons_pressed;

    bool isKeyPressed(int raylib_key_code) const;
    bool isMouseButtonPressed(int raylib_button_code) const;
};

struct InputMap {
    std::unordered_map<std::string, std::vector<Binding>> action_to_keys;

    void bindAction(const std::string& action_name, InputType type, int raylib_key_code);

    bool isActionPressed(const std::string& action_name, const UserInput& userInput) const;
};

class InputPlugin final : public Plugin {

public:
    void build(Application &app) override;
};

}
#pragma once

#include <R-Engine/Types.hpp>

#include <string>
#include <string_view>

/**
 * @brief forward lua types declaration
 */
struct lua_State;
struct luaL_Reg;

namespace r {

namespace lua {

/**
 * @brief type aliases for core Lua C types to use within the C++ wrapper
 */

using State = ::lua_State;
using CFunction = int (*)(State *);
using CFunctionRegistry = ::luaL_Reg;

/**
 * @brief stack interaction utility functions
 */

/**
* @brief Get values from the Lua stack with type safety
* @tparam T The expected C++ type to retrieve from the Lua stack.
* @param L The Lua state.
* @param index The stack index to retrieve the value from.
* @return The value at the specified index, converted to type T.
 */
template<typename T>
T get(State *L, int index);

template<>
bool get<bool>(State *L, int index);
template<>
std::string get<std::string>(State *L, int index);
template<>
std::string_view get<std::string_view>(State *L, int index);

/**
* @brief Push values onto the Lua stack with type safety
* @param L The Lua state.
* @param value The value to push onto the stack.
*/
void push(State *L, bool value);
void push(State *L, const std::string &value);
void push(State *L, std::string_view value);

/**
 * @brief Creates a new global table (module) and populates it with C functions.
 * @param L The Lua state.
 * @param tableName The name of the global table to create (e.g., "Log").
 * @param functions An array of luaL_Reg structs defining the functions for the module.
 */
void register_module(State *L, const std::string &tableName, const CFunctionRegistry functions[]);

}// namespace lua

}// namespace r

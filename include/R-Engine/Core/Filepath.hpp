#pragma once

#include <filesystem>

namespace r {

namespace path {

/**
* @brief check if a file exists at the given path
*/
static inline bool exists(const std::string &path)
{
    return std::filesystem::exists(path);
}

/**
* @brief get the absolute path from a relative path
*/
static inline const std::string get(const std::string &path)
{
    const auto absolute = std::filesystem::absolute(path);

    return absolute.string();
}

}// namespace path

}// namespace r

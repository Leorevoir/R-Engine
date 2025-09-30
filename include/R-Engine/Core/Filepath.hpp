#pragma once

#include <filesystem>

namespace r {

namespace path {

static constexpr inline bool exists(const std::string &path)
{
    return std::filesystem::exists(path);
}

static constexpr inline const std::string get(const std::string &path)
{
    const auto absolute = std::filesystem::absolute(path);

    return absolute.string();
}

}// namespace path

}// namespace r

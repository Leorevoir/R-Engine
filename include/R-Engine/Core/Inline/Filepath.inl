#pragma once

inline bool r::path::exists(const std::string &path)
{
    return std::filesystem::exists(path);
}

inline const std::string r::path::get(const std::string &path)
{
    const auto absolute = std::filesystem::absolute(path);

    return absolute.string();
}

inline std::filesystem::file_time_type r::path::last_write_time(const std::string &path)
{
    return std::filesystem::last_write_time(path);
}

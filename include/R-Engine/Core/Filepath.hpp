#pragma once

#include <filesystem>

namespace r {

namespace path {

/**
* @brief check if a file exists at the given path
*/
static inline bool exists(const std::string &path);

/**
* @brief get the absolute path from a relative path
*/
static inline const std::string get(const std::string &path);

/**
* @brief get the last write time of a file at the given path
*/
static inline std::filesystem::file_time_type last_write_time(const std::string &path);

}// namespace path

}// namespace r

#include "Inline/Filepath.inl"

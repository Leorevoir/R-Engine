#pragma once

#include <R-Engine/Core/FrameTime.hpp>

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

/**
* public
*/

inline void r::Logger::debug([[maybe_unused]] const std::string_view message, [[maybe_unused]] const std::string_view file,
    [[maybe_unused]] int line) noexcept
{
#if defined(ENGINE_DEBUG)
    _emit(message, Level::Debug, file, line);
#endif
}

inline void r::Logger::info(const std::string_view message, const std::string_view file, int line) noexcept
{
    _emit(message, Level::Info, file, line);
}

inline void r::Logger::warn(const std::string_view message, const std::string_view file, int line) noexcept
{
    _emit(message, Level::Warn, file, line);
}

inline void r::Logger::error(const std::string_view message, const std::string_view file, int line) noexcept
{
    _emit(message, Level::Error, file, line);
}

/**
* private
*/

inline void r::Logger::_emit(const std::string_view message, Level level, const std::string_view file, int line) noexcept
{
    const core::TimePoint now = core::SystemClock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    std::ostringstream timestamp;
    timestamp << std::put_time(&tm, "%H:%M:%S");//<< ⏱ hh:mm:ss

    const std::string filename = std::filesystem::path(file).filename().string();

    std::cout << COLOR_BOLD << _level_to_color(level) << "[" << _level_to_string(level) << "]\t" << COLOR_CONTEXT << timestamp.str() << " "
              << filename << ":" << line << COLOR_RESET << " " << COLOR_BOLD << _level_to_color(level) << message << COLOR_RESET << '\n';
}

constexpr std::string_view r::Logger::_level_to_string(Level lvl) noexcept
{
    switch (lvl) {
        case Level::Debug:
            return "DEBUG";
        case Level::Info:
            return "INFO";
        case Level::Warn:
            return "WARN";
        case Level::Error:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

constexpr std::string_view r::Logger::_level_to_color(Level lvl) noexcept
{
    switch (lvl) {
        case Level::Debug:
            return COLOR_DEBUG;
        case Level::Info:
            return COLOR_INFO;
        case Level::Warn:
            return COLOR_WARN;
        case Level::Error:
            return COLOR_ERROR;
        default:
            return COLOR_RESET;
    }
}

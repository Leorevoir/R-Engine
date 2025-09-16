#include <R-Engine/Core/FrameTime.hpp>
#include <R-Engine/Core/Logger.hpp>

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

/**
* public
*/

void r::Logger::debug([[maybe_unused]] const std::string_view message, [[maybe_unused]] const std::string_view file,
    [[maybe_unused]] int line) noexcept
{
#if defined(ENGINE_DEBUG)
    _emit(message, Level::Debug, file, line);
#endif
}

void r::Logger::info(const std::string_view message, const std::string_view file, int line) noexcept
{
    _emit(message, Level::Info, file, line);
}

void r::Logger::warn(const std::string_view message, const std::string_view file, int line) noexcept
{
    _emit(message, Level::Warn, file, line);
}

void r::Logger::error(const std::string_view message, const std::string_view file, int line) noexcept
{
    _emit(message, Level::Error, file, line);
}

/**
* private
*/

void r::Logger::_emit(const std::string_view message, Level level, const std::string_view file, int line) noexcept
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

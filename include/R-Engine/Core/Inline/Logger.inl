#pragma once

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

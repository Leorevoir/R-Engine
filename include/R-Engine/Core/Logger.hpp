#include <string_view>

namespace r {

class Logger
{
    public:
        enum class Level { Debug, Info, Warn, Error };

        static void debug(const std::string_view message, const std::string_view file = __builtin_FILE(),
            int line = __builtin_LINE()) noexcept;

        static void info(const std::string_view message, const std::string_view file = __builtin_FILE(),
            int line = __builtin_LINE()) noexcept;

        static void warn(const std::string_view message, const std::string_view file = __builtin_FILE(),
            int line = __builtin_LINE()) noexcept;

        static void error(const std::string_view message, const std::string_view file = __builtin_FILE(),
            int line = __builtin_LINE()) noexcept;

    private:
        static constexpr std::string_view COLOR_RESET = "\033[0m";
        static constexpr std::string_view COLOR_BOLD = "\033[1m";
        static constexpr std::string_view COLOR_DEBUG = "\033[38;5;188m";
        static constexpr std::string_view COLOR_INFO = "\033[38;5;183m";
        static constexpr std::string_view COLOR_WARN = "\033[38;5;216m";
        static constexpr std::string_view COLOR_ERROR = "\033[38;5;203m";
        static constexpr std::string_view COLOR_CONTEXT = "\033[38;5;138m";

        static constexpr std::string_view _level_to_string(Level lvl) noexcept;
        static constexpr std::string_view _level_to_color(Level lvl) noexcept;

        static void _emit(const std::string_view message, Level level, const std::string_view file, int line) noexcept;
};

}// namespace r

#include "Inline/Logger.inl"

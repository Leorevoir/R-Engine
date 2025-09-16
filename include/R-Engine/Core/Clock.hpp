#pragma once

#include <R-Engine/Core/FrameTime.hpp>

namespace r {

namespace core {

class Clock
{
    public:
        constexpr Clock() = default;
        constexpr ~Clock() = default;

        void tick() noexcept;
        const FrameTime &frame() const noexcept;

    private:
        core::FrameTime _frame{};
        core::LastTime _last{};
};

}// namespace core

}// namespace r

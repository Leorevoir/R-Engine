#pragma once

#include <R-Engine/Core/FrameTime.hpp>
#include <R-Engine/R-EngineExport.hpp>

namespace r {

namespace core {

class R_ENGINE_API Clock
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

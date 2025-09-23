#pragma once

#include <R-Engine/Types.hpp>

namespace r {

template<typename T>
class Handle
{
    public:
        static constexpr u32 InvalidId = 0;

        explicit constexpr Handle(const u32 id = InvalidId) noexcept;

        bool valid() const noexcept;
        u32 id() const noexcept;

        bool operator==(const Handle<T> &other) const noexcept;
        bool operator!=(const Handle<T> &other) const noexcept;

    private:
        u32 _id;
};

#include "Inline/Handle.inl"

}// namespace r

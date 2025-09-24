#pragma once

#include <R-Engine/Types.hpp>

namespace r::assets {

template<typename T>
class Handle
{
    public:
        using IdType = u32;
        static constexpr IdType InvalidId = 0;

        constexpr Handle(const IdType id = InvalidId) noexcept;

        /**
        * @brief check if handle is valid do i really need to explain....
        */
        bool valid() const noexcept;

        /**
        * @brief get the id quite litteral right....
        */
        IdType id() const noexcept;

        bool operator==(const Handle<T> &other) const noexcept;
        bool operator!=(const Handle<T> &other) const noexcept;

    private:
        IdType _id;
};

#include "Inline/Handle.inl"

}// namespace r::assets

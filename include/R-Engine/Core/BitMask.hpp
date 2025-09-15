#pragma once

#include <R-Engine/Types.hpp>

#include <initializer_list>
#include <vector>

namespace r {

namespace core {

class BitMask
{
    public:
        constexpr BitMask() = default;
        constexpr ~BitMask() = default;

        explicit BitMask(u64 bit_count);
        explicit BitMask(std::initializer_list<bool> values);

        void set(u64 index, bool value = true);
        void reset(u64 index);

        BitMask operator~() const;
        BitMask operator&(const BitMask &BitMask) const;
        BitMask operator|(const BitMask &BitMask) const;
        BitMask operator^(const BitMask &BitMask) const;
        BitMask operator<<(std::size_t shift) const;
        BitMask operator>>(std::size_t shift) const;
        BitMask &operator&=(const BitMask &BitMask);
        BitMask &operator|=(const BitMask &BitMask);
        BitMask &operator^=(const BitMask &BitMask);
        BitMask &operator<<=(std::size_t shift);
        BitMask &operator>>=(std::size_t shift);

        bool operator[](std::size_t index) const;
        bool operator==(const BitMask &BitMask) const;
        bool operator!=(const BitMask &BitMask) const;

        bool empty() const;

    private:
        std::vector<bool> _bits{};
};

}// namespace core

}// namespace r

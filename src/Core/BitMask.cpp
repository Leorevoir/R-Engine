#include <R-Engine/Core/BitMask.hpp>
#include <algorithm>

/**
 * public
 */

r::core::BitMask::BitMask(u64 bit_count) : _bits(bit_count, false)
{
    /* __ctor__ */
}

r::core::BitMask::BitMask(std::initializer_list<bool> values) : _bits(values)
{
    /* __ctor__ */
}

/**
 * setters
 */

void r::core::BitMask::set(u64 index, bool value)
{
    if (index >= _bits.size()) {
        _bits.resize(index + 1);
    }
    _bits[index] = value;
}

void r::core::BitMask::reset(u64 index)
{
    if (index < _bits.size()) {
        _bits[index] = false;
    }
}

/**
* operators
*/

r::core::BitMask r::core::BitMask::operator~() const
{
    r::core::BitMask res = *this;

    for (const auto &bit : res._bits) {
        bit = !bit;
    }
    return res;
}

r::core::BitMask r::core::BitMask::operator&(const r::core::BitMask &other) const
{
    r::core::BitMask res(std::min(_bits.size(), other._bits.size()));

    for (u64 i = 0; i < res._bits.size(); ++i) {
        res._bits[i] = _bits[i] && other._bits[i];
    }
    return res;
}

r::core::BitMask r::core::BitMask::operator|(const r::core::BitMask &other) const
{
    r::core::BitMask res(std::min(_bits.size(), other._bits.size()));

    for (u64 i = 0; i < res._bits.size(); ++i) {
        res._bits[i] = _bits[i] || other._bits[i];
    }
    return res;
}

r::core::BitMask r::core::BitMask::operator^(const r::core::BitMask &other) const
{
    r::core::BitMask res(std::min(_bits.size(), other._bits.size()));

    for (u64 i = 0; i < res._bits.size(); ++i) {
        res._bits[i] = _bits[i] != other._bits[i];
    }
    return res;
}

r::core::BitMask r::core::BitMask::operator<<(std::size_t shift) const
{
    r::core::BitMask res = *this;

    res <<= shift;
    return res;
}

r::core::BitMask r::core::BitMask::operator>>(std::size_t shift) const
{
    r::core::BitMask res = *this;
    res >>= shift;
    return res;
}

r::core::BitMask &r::core::BitMask::operator&=(const r::core::BitMask &other)
{
    for (u64 i = 0; i < std::min(_bits.size(), other._bits.size()); ++i) {
        _bits[i] = _bits[i] && other._bits[i];
    }
    return *this;
}

r::core::BitMask &r::core::BitMask::operator|=(const r::core::BitMask &other)
{
    for (u64 i = 0; i < std::min(_bits.size(), other._bits.size()); ++i) {
        _bits[i] = _bits[i] || other._bits[i];
    }
    return *this;
}

r::core::BitMask &r::core::BitMask::operator^=(const r::core::BitMask &other)
{
    for (u64 i = 0; i < std::min(_bits.size(), other._bits.size()); ++i) {
        _bits[i] = _bits[i] != other._bits[i];
    }
    return *this;
}

r::core::BitMask &r::core::BitMask::operator<<=(std::size_t shift)
{
    _bits.resize(_bits.size() + shift);
    return *this;
}

r::core::BitMask &r::core::BitMask::operator>>=(std::size_t shift)
{
    if (shift >= _bits.size()) {
        _bits.clear();
    } else {
        _bits.erase(_bits.begin(), _bits.begin() + static_cast<std::ptrdiff_t>(shift));
    }
    return *this;
}

bool r::core::BitMask::operator[](std::size_t index) const
{
    return _bits[index];
}

bool r::core::BitMask::operator==(const r::core::BitMask &other) const
{
    return std::equal(_bits.cbegin(), _bits.cend(), other._bits.cbegin());
}

bool r::core::BitMask::operator!=(const r::core::BitMask &other) const
{
    return !(*this == other);
}

/**
* helpers
*/

bool r::core::BitMask::empty() const
{
    return std::all_of(_bits.cbegin(), _bits.cend(), [](bool bit) -> bool { return !bit; });
}

#pragma once

#define R_ENUM_FLAGABLE(EnumType)                                                                                                          \
    constexpr inline EnumType operator|(EnumType a, EnumType b) noexcept                                                                   \
    {                                                                                                                                      \
        return static_cast<EnumType>(static_cast<std::underlying_type_t<EnumType>>(a) | static_cast<std::underlying_type_t<EnumType>>(b)); \
    }                                                                                                                                      \
                                                                                                                                           \
    constexpr inline EnumType &operator|=(EnumType &a, EnumType b) noexcept                                                                \
    {                                                                                                                                      \
        a = a | b;                                                                                                                         \
        return a;                                                                                                                          \
    }                                                                                                                                      \
                                                                                                                                           \
    constexpr inline EnumType operator&(EnumType a, EnumType b) noexcept                                                                   \
    {                                                                                                                                      \
        return static_cast<EnumType>(static_cast<std::underlying_type_t<EnumType>>(a) & static_cast<std::underlying_type_t<EnumType>>(b)); \
    }                                                                                                                                      \
                                                                                                                                           \
    constexpr inline EnumType &operator&=(EnumType &a, EnumType b) noexcept                                                                \
    {                                                                                                                                      \
        a = a & b;                                                                                                                         \
        return a;                                                                                                                          \
    }                                                                                                                                      \
                                                                                                                                           \
    constexpr inline EnumType operator~(EnumType a) noexcept                                                                               \
    {                                                                                                                                      \
        return static_cast<EnumType>(~static_cast<std::underlying_type_t<EnumType>>(a));                                                   \
    }

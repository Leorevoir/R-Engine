#pragma once

#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wundef"
    #pragma clang diagnostic ignored "-Wmissing-field-initializers"
    #pragma clang diagnostic ignored "-Wswitch-default"
#elif defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wundef"
    #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    #pragma GCC diagnostic ignored "-Wswitch-default"
#endif
#include <raylib.h>
#if defined(__clang__)
    #pragma clang diagnostic pop
#elif defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif

#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
    #ifdef R_ENGINE_BUILDING_EXPORT
        #ifdef __GNUC__
            #define R_ENGINE_API __attribute__((dllexport))
        #else
            #define R_ENGINE_API __declspec(dllexport)
        #endif
    #else
        #ifdef __GNUC__
            #define R_ENGINE_API __attribute__((dllimport))
        #else
            #define R_ENGINE_API __declspec(dllimport)
        #endif
    #endif
    #define R_ENGINE_LOCAL
#else
    #if __GNUC__ >= 4
        #define R_ENGINE_API __attribute__((visibility("default")))
        #define R_ENGINE_LOCAL __attribute__((visibility("hidden")))
    #else
        #define R_ENGINE_API
        #define R_ENGINE_LOCAL
    #endif
#endif

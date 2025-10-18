########################################
# RayLib
########################################

if (APPLE)
    set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "Suppress dev warnings")
endif ()

########################################

add_subdirectory(external/raylib EXCLUDE_FROM_ALL)

########################################

if (APPLE)
    set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 0 CACHE INTERNAL "Re-enable dev warnings")
endif()

########################################

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(raylib PRIVATE -Wno-stringop-overflow -Wno-undef)
    target_link_options(raylib PRIVATE -Wno-stringop-overflow -Wno-undef)
elseif (NOT MSVC)
    target_compile_options(raylib PRIVATE -Wno-undef)
    target_link_options(raylib PRIVATE -Wno-undef)
endif ()

########################################
# Lua
########################################

add_subdirectory(external/lua EXCLUDE_FROM_ALL)

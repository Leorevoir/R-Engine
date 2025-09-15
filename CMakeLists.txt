cmake_minimum_required(VERSION 3.16)
project(r-engine LANGUAGES C CXX VERSION 0.0.1)

########################################

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

########################################

include(1_Options)
include(2_Optimizations)
include(3_External)
include(4_Sources)

########################################

add_executable(r-engine ${SRC_ENGINE})

target_include_directories(r-engine PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external
)

target_link_libraries(r-engine PRIVATE
    raylib
)

########################################

include(5_Warnings)
apply_compiler_warnings(r-engine)

########################################

include(6_Linker)
apply_linker_optimizations(r-engine)

########################################

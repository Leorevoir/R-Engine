cmake_minimum_required(VERSION 3.16)
project(r-engine LANGUAGES C CXX VERSION 0.0.1)

########################################

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

########################################

include(1_Options)
include(2_Optimizations)
include(3_External)
include(4_Sources)
include(5_Warnings)
include(6_Linker)
include(7_Target)
include(8_UnitTests)

########################################

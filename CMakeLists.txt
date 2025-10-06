cmake_minimum_required(VERSION 3.16)
project(r-engine LANGUAGES C CXX VERSION 0.0.1)

########################################

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

########################################

include(1_Options)
include(2_Optimizations)
include(3_External)
include(4_Sources)

########################################
# R-Engine lib
########################################

add_library(r-engine SHARED ${SRC_ENGINE})
target_include_directories(r-engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
)
target_link_libraries(r-engine PUBLIC
    raylib
)
target_compile_definitions(r-engine PRIVATE
    R_ENGINE_BUILDING_EXPORTS=1
)

########################################
# R-Engine unit tests
########################################

if(ENABLE_TESTS)

    add_executable(unit_tests ${SRC_TESTS})
    set_target_properties(unit_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
        target_compile_options(unit_tests PRIVATE -fprofile-instr-generate -fcoverage-mapping)
        target_link_options(unit_tests PRIVATE -fprofile-instr-generate -fcoverage-mapping)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(unit_tests PRIVATE -fprofile-arcs -ftest-coverage)
        target_link_options(unit_tests PRIVATE -fprofile-arcs -ftest-coverage)
    endif()

    target_compile_definitions(unit_tests PRIVATE ENGINE_TESTS=1)
    target_link_libraries(unit_tests PRIVATE criterion r-engine)

    enable_testing()
    add_test(NAME unit_tests COMMAND unit_tests)

    target_include_directories(unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

endif()

add_custom_target(tests_run
    COMMENT "Running unit tests"
    COMMAND ${CMAKE_COMMAND} -DBUILD_TESTS=ON ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target unit_tests
    COMMAND ${CMAKE_BINARY_DIR}/unit_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)


########################################

include(5_Warnings)
apply_compiler_warnings(r-engine)

########################################

include(6_Linker)
apply_linker_optimizations(r-engine)

########################################

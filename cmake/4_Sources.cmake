file(GLOB_RECURSE SRC_ENGINE "src/*.cpp")
set(INCLUDE_ENGINE
    "${CMAKE_SOURCE_DIR}/include"
)

if(ENABLE_TESTS)
    file(GLOB_RECURSE SRC_TESTS "tests/*.cpp")
    set(INCLUDE_TESTS
        "${CMAKE_SOURCE_DIR}/tests"
    )
endif()

file(GLOB_RECURSE SRC_EXAMPLE "examples/*.cpp")
set(INCLUDE_EXAMPLES
    "${CMAKE_SOURCE_DIR}/examples"
    )

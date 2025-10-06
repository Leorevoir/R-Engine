file(GLOB_RECURSE SRC_ENGINE "src/*.cpp")
set(INCLUDE_ENGINE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

if(ENABLE_TESTS)
    file(GLOB_RECURSE SRC_TESTS "tests/*.cpp")
    set(INCLUDE_TESTS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
    )
endif()

file(GLOB EXAMPLE_SUBDIRS LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/examples/*")

set(EXAMPLE_TARGETS "")

foreach(EXAMPLE_DIR ${EXAMPLE_SUBDIRS})
    if(IS_DIRECTORY ${EXAMPLE_DIR})
        get_filename_component(EXAMPLE_NAME ${EXAMPLE_DIR} NAME)
        set(TARGET_NAME "r-engine__${EXAMPLE_NAME}")

        file(GLOB_RECURSE EXAMPLE_SRCS "${EXAMPLE_DIR}/*.cpp")

        if(EXAMPLE_SRCS)
            add_executable(${TARGET_NAME} ${EXAMPLE_SRCS})
            target_include_directories(${TARGET_NAME} PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/include
            )
            target_link_libraries(${TARGET_NAME} PRIVATE
                r-engine
            )
            list(APPEND EXAMPLE_TARGETS ${TARGET_NAME})
            message(STATUS "INFO: configured example target: ${TARGET_NAME}")
        endif()
    endif()
endforeach()

if(EXAMPLE_TARGETS)
    add_custom_target(examples DEPENDS ${EXAMPLE_TARGETS})
endif()

set(INCLUDE_EXAMPLES
    "${CMAKE_CURRENT_SOURCE_DIR}/examples"
)

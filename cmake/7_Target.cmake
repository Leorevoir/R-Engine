add_library(r-engine SHARED ${SRC_ENGINE})

########################################

target_include_directories(r-engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
)
find_package(OpenSSL REQUIRED)
target_link_libraries(r-engine PUBLIC
    raylib
    lua_lib
    OpenSSL::SSL
    OpenSSL::Crypto
    r-type_network
)
target_compile_definitions(r-engine PRIVATE
    R_ENGINE_BUILDING_EXPORTS=1
)

########################################

apply_compiler_warnings(r-engine)
apply_linker_optimizations(r-engine)

########################################

add_custom_command(
    TARGET r-engine POST_BUILD
    COMMENT "Copying networking shared libraries..."
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:r-type_network>
        $<TARGET_FILE_DIR:r-engine>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:r-type_network_core>
        $<TARGET_FILE_DIR:r-engine>
    VERBATIM
)

if(NOT MSVC)
    add_custom_command(
        TARGET r-engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:r-type_network_subplatform>
            $<TARGET_FILE_DIR:r-engine>
        VERBATIM
    )
endif()

########################################

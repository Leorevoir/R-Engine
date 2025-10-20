add_library(r-engine SHARED ${SRC_ENGINE})

########################################

target_include_directories(r-engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
)
target_link_libraries(r-engine PUBLIC
    raylib
    lua_lib
)
target_compile_definitions(r-engine PRIVATE
    R_ENGINE_BUILDING_EXPORTS=1
)

########################################

apply_compiler_warnings(r-engine)
apply_linker_optimizations(r-engine)

########################################

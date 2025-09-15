add_subdirectory(external/raylib EXCLUDE_FROM_ALL)
target_compile_options(raylib PRIVATE -Wno-stringop-overflow -Wno-undef) # raylib triggers these warnings

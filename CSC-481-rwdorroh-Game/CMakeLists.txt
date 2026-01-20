cmake_minimum_required(VERSION 3.10)
project(Game)

set(CMAKE_CXX_STANDARD 17)

# Game source files
add_executable(game
    game/main.cpp
    game/Player.cpp
    game/Auto.cpp
    game/Static.cpp
 "game/Actions.h")
# Include the engine headers (you already set up include/engine/ in the engine)
target_include_directories(game PRIVATE
    ${CMAKE_SOURCE_DIR}/CSC-481-Engine-Design/include
)

# Link the game against engine library
target_link_libraries(game PRIVATE engine_lib)

# Copy asset files to the build directory
add_custom_command(
    TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:game>/assets
)

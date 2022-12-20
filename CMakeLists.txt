cmake_minimum_required(VERSION 3.15)
project(HMI)

set(CMAKE_CXX_STANDARD 14)

add_executable(
        HMI
        Color.h
        Game.cpp
        Game.h
        Graph.h
        Graph.cpp
        Node.h
        Node.cpp
        Level.h
        Level.cpp
        main.cpp
        Graphics.h Graphics.cpp)

include_directories(/usr/include/stb)
target_link_libraries(HMI EGL GLESv2 X11 png stb)


add_custom_target(copy_textures COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/textures ${CMAKE_CURRENT_BINARY_DIR}/textures)
add_custom_target(copy_shader COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shader ${CMAKE_CURRENT_BINARY_DIR}/shader)
add_dependencies(HMI copy_textures copy_shader)
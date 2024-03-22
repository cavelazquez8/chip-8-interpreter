cmake_minimum_required(VERSION 3.2)
project(chip8 LANGUAGES CXX)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(chip8 main.cpp chip8.cpp)
target_link_libraries(chip8 ${SDL2_LIBRARIES})


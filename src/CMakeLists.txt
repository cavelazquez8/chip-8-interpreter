cmake_minimum_required(VERSION 3.14)
project(chip8 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIR})

set(IMGUI_SOURCES
  imgui/imgui.cpp
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui_impl_sdl2.cpp
  imgui/imgui_impl_opengl3.cpp
)
# Create a library for the core chip8 functionality
add_library(chip8_core STATIC chip8.cpp)
target_include_directories(chip8_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Create the main executable
add_executable(chip8 main_gui.cpp gui_application.cpp disassembler.cpp ${IMGUI_SOURCES})
target_include_directories(chip8 PRIVATE imgui)  # Include the IMGUI headers
target_link_libraries(chip8 chip8_core)
#target_link_libraries(chip8 ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})
target_link_libraries(chip8 ${SDL2_LIBRARIES})
target_link_libraries(chip8 ${OPENGL_LIBRARIES})


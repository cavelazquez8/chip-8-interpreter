cmake_minimum_required(VERSION 3.2)
project(chip8 LANGUAGES CXX)

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
add_executable(chip8 main.cpp chip8.cpp ${IMGUI_SOURCES})
target_include_directories(chip8 PRIVATE imgui)  # Include the IMGUI headers
#target_link_libraries(chip8 ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})
target_link_libraries(chip8 ${SDL2_LIBRARIES})
target_link_libraries(chip8 ${OPENGL_LIBRARIES})


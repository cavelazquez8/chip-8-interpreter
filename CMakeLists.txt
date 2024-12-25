cmake_minimum_required(VERSION 3.14)
project(Chip8 LANGUAGES CXX)

enable_testing()

option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

# If coverage is enabled, add coverage flags
if(ENABLE_COVERAGE)
  message(STATUS "Coverage build enabled")

  # NOTE: These flags are for GCC/Clang. Adjust if you're on another compiler.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -O0 -g")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage -O0 -g")
endif()

# Add subdirectories
add_subdirectory(src)
add_subdirectory(tests)


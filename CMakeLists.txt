cmake_minimum_required(VERSION 3.14)

project(provincial)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(psl
src/psl/iostream.cpp
src/psl/fstream.cpp
src/psl/string.cpp
src/psl/chrono.cpp
src/psl/memory.cpp
src/psl/system.cpp
src/psl/check.cpp
)
target_include_directories(psl PUBLIC src/)

add_executable(game
src/contrib/glad/glad.c
src/pine/vecmath.cpp
src/pine/fileio.cpp
src/pine/noise.cpp
src/pine/log.cpp
src/main.cpp
)
target_compile_options(game PRIVATE -Wall -Wextra -pedantic)
target_include_directories(game PRIVATE src src/contrib)

find_package(glfw3 REQUIRED ON)
target_link_libraries(game PRIVATE psl glfw3)

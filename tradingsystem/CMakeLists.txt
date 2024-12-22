# Minimum CMake version
cmake_minimum_required(VERSION 3.10)

# Project name and language
project(tradingsystem LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set Export Compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Specify the GCC compiler
set(CMAKE_CXX_COMPILER "/usr/bin/g++-13")
set(CMAKE_C_COMPILER "/usr/bin/gcc-13")

# Include directories for header files
include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)

# Add executable (only source files are needed here)
add_executable(tradingsystem src/main.cpp)

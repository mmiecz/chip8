cmake_minimum_required(VERSION 3.5)
project(chip8)
FIND_PACKAGE( Boost 1.61.0 )
include_directories(${Boost_INCLUDE_DIRS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
message("system: ${CMAKE_SYSTEM_PREFIX_PATH}")

add_definitions(-DDEBUG)

set(SOURCE_FILES main.cpp instruction_set.h)
add_executable(chip8 ${SOURCE_FILES})
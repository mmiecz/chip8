cmake_minimum_required(VERSION 3.5)
project(chip8)
FIND_PACKAGE( Boost 1.61.0 )
include_directories(${Boost_INCLUDE_DIRS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
INCLUDE(FindPkgConfig)

PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2TTF REQUIRED SDL2_ttf)

add_definitions(-DDEBUG)

set(SOURCE_FILES main.cpp )
add_executable(chip8 ${SOURCE_FILES})
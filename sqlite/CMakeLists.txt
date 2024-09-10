#Native build on Linux

cmake_minimum_required(VERSION 3.10)
project(sqlite)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set( CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused -g" )
set( CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused -g" )
include_directories(${CMAKE_PROGRAM_PATH}/)
add_library(sqlite shell.c sqlite3.c )
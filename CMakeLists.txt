cmake_minimum_required(VERSION 3.20)
project(untitled)

set(CMAKE_CXX_STANDARD 11)

add_executable(untitled signals.h signals.cpp Commands.h Commands.cpp smash.cpp)

cmake_minimum_required(VERSION 3.12)
project(c C)

set(CMAKE_C_STANDARD 11)

add_definitions(-Wno-implicit-function-declaration)

add_executable(copy copy.c)
add_executable(env env.c)
add_executable(list list.c)

cmake_minimum_required(VERSION 3.12)
project(c C)

set(CMAKE_C_STANDARD 11)

add_definitions(-Wno-implicit-function-declaration)

add_executable(copy cmd/copy.c)
add_executable(strip cmd/strip.c)
add_executable(crappy_sh cmd/crappy_sh.c)
add_executable(env cmd/env.c)
add_executable(list cmd/list.c)

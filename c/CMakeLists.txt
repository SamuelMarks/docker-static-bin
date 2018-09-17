cmake_minimum_required(VERSION 3.0)
project(c VERSION 0.0.9)

set(CMAKE_C_STANDARD 11)

configure_file(lib/version.h.in version.h @ONLY)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-Wno-implicit-function-declaration)

add_executable(copy cmd/copy.c)
add_executable(strip cmd/strip.c)
add_executable(crappy_sh cmd/crappy_sh.c)
add_executable(env cmd/env.c)
add_executable(list cmd/list.c)

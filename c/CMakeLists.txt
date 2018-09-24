cmake_minimum_required(VERSION 3.0)
project(c VERSION 0.0.10)

set(CMAKE_C_STANDARD 11)

configure_file(lib/version.h.in version.h @ONLY)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-Wno-implicit-function-declaration)
if (NOT APPLE)
    add_definitions('-static')
endif ()
add_executable(main cmd/main.c cmd/copy.c cmd/crappy_sh.c cmd/env.c cmd/list.c)

cmake_minimum_required(VERSION 3.12)
project(cc)

set(CMAKE_CXX_STANDARD 11)

add_definitions(-O3)

# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

message("CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
message("CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")

add_executable(crappy_sh crappy_sh.cc)

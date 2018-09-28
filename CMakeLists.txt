cmake_minimum_required(VERSION 3.0)
project(docker_static_bin VERSION 0.0.10)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(CMOCKA REQUIRED)

include(CTest)
if (CMOCKA_FOUND)
    include_directories(${CMOCKA_INCLUDE_DIRS})
    # include(${CMOCKA_INCLUDE_DIRS})
    # find_package(cmocha REQUIRED)
else()
    message(could not find CMocka, did you run `conan install`?)
endif()

set(CMAKE_C_STANDARD 11)

configure_file(lib/version.h.in version.h @ONLY)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-Wno-implicit-function-declaration)

add_executable(copy cmd/copy.c)
add_executable(strip cmd/strip.c)
add_executable(crappy_sh cmd/crappy_sh.c)
add_executable(env cmd/env.c)
add_executable(list cmd/list.c)

enable_testing()


#file(GLOB files "test/test_*.c")
#
#foreach(file ${files})
#    string(REGEX REPLACE "(^.*/|\\.[^.]*$)" "" file_without_ext ${file})
#    add_executable(${file_without_ext} ${file})
#    target_link_libraries(${file_without_ext} ${PROJECT_LIBS})
#    add_test(${file_without_ext} ${file_without_ext})
#    set_tests_properties(${file_without_ext}
#            PROPERTIES
#            PASS_REGULAR_EXPRESSION "Test passed")
#    set_tests_properties(${file_without_ext}
#            PROPERTIES
#            FAIL_REGULAR_EXPRESSION "(Exception|Test failed)")
#    set_tests_properties(${file_without_ext}
#            PROPERTIES
#            TIMEOUT 120)
#endforeach()

add_executable(test_crappy_sh test/test_crappy_sh.c)

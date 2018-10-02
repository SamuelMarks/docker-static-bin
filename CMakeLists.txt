cmake_minimum_required(VERSION 3.0)
project(docker_static_bin VERSION 0.0.10)

set(CMAKE_C_STANDARD 11)

configure_file(lib/version.h.in version.h @ONLY)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-Wno-implicit-function-declaration)

if (NOT "$ENV{TEST_ENABLED}" STREQUAL "")
    set(TEST_ENABLED "$ENV{TEST_ENABLED}" CACHE INTERNAL "Copied from environment variable")
else ()
    set(TEST_ENABLED 1)
endif ()

if (TEST_ENABLED)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup(KEEP_RPATHS)
    enable_testing()

    find_package(CMOCKA REQUIRED)
    include(CTest)
    if (CMOCKA_FOUND)
        include_directories("${CONAN_INCLUDE_DIRS_CMOCKA}/include")
        add_executable(test_crappy_sh test/test_crappy_sh.c)
        add_dependencies(test_crappy_sh test_crappy_sh)
        add_test(test_crappy_sh test_crappy_sh)
        target_link_libraries(test_crappy_sh cmocka)
    else ()
        message(could not find CMocka, did you run `conan install`?)
    endif ()
endif ()

add_executable(copy cmd/copy.c)
add_executable(strip cmd/strip.c)
add_executable(crappy_sh cmd/crappy_sh.c)
add_executable(env cmd/env.c)
add_executable(list cmd/list.c)


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

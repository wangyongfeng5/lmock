cmake_minimum_required(VERSION 3.10)
project(lmock)

set(CMAKE_CXX_STANDARD 11)

add_library(lmock STATIC
    lmock.cpp)

add_executable(test test.cpp)

target_link_libraries(test PRIVATE lmock)
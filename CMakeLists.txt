cmake_minimum_required(VERSION 3.2)
project(matasano)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(base64 1-1-base64.cpp)
add_executable(fixed-xor 1-2-fixed-xor.cpp)
add_executable(single-byte-xor 1-3-single-byte-xor.cpp)
add_executable(detect-single-byte-xor 1-4-detect-single-byte-xor.cpp)

cmake_minimum_required(VERSION 3.3)
project(samples_echo_server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.c)
add_executable(samples_echo_server ${SOURCE_FILES})
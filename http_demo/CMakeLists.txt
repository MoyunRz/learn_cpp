cmake_minimum_required(VERSION 3.25)
project(http_demo)

set(CMAKE_CXX_STANDARD 17)

add_executable(http_demo main.cpp httplib.h)

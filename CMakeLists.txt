cmake_minimum_required(VERSION 3.5)
project(mpi_benchmark)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(mpi_benchmark ${SOURCE_FILES})
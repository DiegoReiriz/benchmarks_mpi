cmake_minimum_required(VERSION 3.5)
project(mpi_benchmark)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

SET(CMAKE_C_COMPILER mpicc)
set(SOURCE_FILES main.c)
add_executable(mpi_benchmark ${SOURCE_FILES})
cmake_minimum_required(VERSION 3.20)
project(MY_PROJECT)

set(CMAKE_CXX_STANDARD 17)

add_executable(zadeanie1 main.cpp)

find_package(MPI REQUIRED)

add_executable(zadanie1 main.cpp)
# Add libraries for code completion and compiling
target_link_libraries(zadanie1 PRIVATE MPI::MPI_CXX)
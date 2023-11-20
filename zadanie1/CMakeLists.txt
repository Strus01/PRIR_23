cmake_minimum_required(VERSION 3.20)
project(zadanie1)

set(CMAKE_CXX_STANDARD 17)

find_package(MPI REQUIRED)

add_executable(zadanie1 Alloc.cpp Life.cpp LifeSequentialImplementation.cpp Main.cpp Rules.cpp SimpleRules.cpp LifeParallelImplementation.cpp)

# Add libraries for code completion and compiling
target_link_libraries(zadanie1 PUBLIC MPI::MPI_CXX)
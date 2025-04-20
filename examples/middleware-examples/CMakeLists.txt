cmake_minimum_required(VERSION 3.10)
project(middleware-examples)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(middleware-examples main.cpp)

# Link with the Boson library
target_link_libraries(middleware-examples PUBLIC boson)
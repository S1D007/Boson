cmake_minimum_required(VERSION 3.10)
project(basic-api)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(basic-api main.cpp)

# Link with the Boson library
target_link_libraries(basic-api PUBLIC boson)
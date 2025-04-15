cmake_minimum_required(VERSION 3.10)
project(json-example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(json-example main.cpp)

# Link with the Boson library
target_link_libraries(json-example PUBLIC boson)
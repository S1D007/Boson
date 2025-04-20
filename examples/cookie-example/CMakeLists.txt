cmake_minimum_required(VERSION 3.10)
project(cookie_example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cookie_example main.cpp)
target_link_libraries(cookie_example PRIVATE boson)
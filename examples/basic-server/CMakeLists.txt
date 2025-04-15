cmake_minimum_required(VERSION 3.12)
project(basic-server)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(basic-server main.cpp)

# Link with Boson library
target_link_libraries(basic-server PRIVATE boson)

# Include directories
target_include_directories(basic-server PRIVATE ${CMAKE_SOURCE_DIR}/../../include)
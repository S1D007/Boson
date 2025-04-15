cmake_minimum_required(VERSION 3.14)
project({{.ProjectName}} VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boson package
find_package(Boson REQUIRED)

# Collect source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS 
    "src/*.hpp"
    "src/controllers/*.hpp"
    "src/models/*.hpp"
    "src/middleware/*.hpp"
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link against Boson library
target_link_libraries(${PROJECT_NAME} PRIVATE Boson::Boson)

# Copy static files to build directory if they exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/public")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/public DESTINATION ${CMAKE_BINARY_DIR})
endif()
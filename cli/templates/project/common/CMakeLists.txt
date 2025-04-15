cmake_minimum_required(VERSION 3.14)
project({{.ProjectName}} VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set BOSON_ROOT to ~/.boson by default, allow override
if(NOT DEFINED BOSON_ROOT)
    set(BOSON_ROOT $ENV{HOME}/.boson)
endif()

# Add Boson include and lib directories
include_directories(${BOSON_ROOT}/include)
link_directories(${BOSON_ROOT})

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

# Include project src directory
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link against Boson static library
target_link_libraries(${PROJECT_NAME} PRIVATE boson)

# Copy static files to build directory if they exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/public")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/public DESTINATION ${CMAKE_BINARY_DIR})
endif()
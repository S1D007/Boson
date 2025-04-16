cmake_minimum_required(VERSION 3.14)
project({{.ProjectName}} VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boson package
find_package(Boson REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Collect source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link against Boson
target_link_libraries(${PROJECT_NAME} PRIVATE Boson::Boson)

# Copy public directory to build directory if it exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/public")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/public DESTINATION ${CMAKE_BINARY_DIR})
endif()
cmake_minimum_required(VERSION 3.14)
project(Boson VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BOSON_BUILD_EXAMPLES "Build example applications" ON)
option(BOSON_WITH_SQLITE "Enable SQLite database support" OFF)
option(BUILD_TESTS "Build tests" OFF)

# Include directories
include_directories(include)

# Add compile definitions
if(BOSON_WITH_SQLITE)
    add_compile_definitions(BOSON_WITH_SQLITE)
endif()

# Add the core library
add_subdirectory(src)

# Optionally build examples
if(BOSON_BUILD_EXAMPLES)
    add_subdirectory(examples/library-management-app)
    add_subdirectory(examples/basic-server)
    add_subdirectory(examples/json-example)
    add_subdirectory(examples/basic-api)
endif()

# Optionally build tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install targets
install(
    DIRECTORY include/boson
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# Generate package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/BosonConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Configure package configuration file
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/BosonConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/BosonConfig.cmake"
    @ONLY
)

# Install package configuration
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/BosonConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/BosonConfigVersion.cmake"
    DESTINATION lib/cmake/Boson
)

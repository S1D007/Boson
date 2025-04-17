cmake_minimum_required(VERSION 3.10)
project({{.ProjectName}} VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(HOME_DIR "$ENV{USERPROFILE}")
else()
    set(HOME_DIR "$ENV{HOME}")
endif()

set(BOSON_DIR "${HOME_DIR}/.boson")
set(BOSON_INCLUDE_DIR "${BOSON_DIR}/include")
set(BOSON_LIB_PATH_UNIX "${BOSON_DIR}/libboson.a")
set(BOSON_LIB_PATH_WIN "${BOSON_DIR}/libboson.lib")

file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.hpp"
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${BOSON_INCLUDE_DIR}"
)

if(WIN32)
    if(EXISTS "${BOSON_LIB_PATH_WIN}")
        target_link_libraries(${PROJECT_NAME} PUBLIC "${BOSON_LIB_PATH_WIN}")
    else()
        message(WARNING "Boson library not found at ${BOSON_LIB_PATH_WIN}")
    endif()
else()
    if(EXISTS "${BOSON_LIB_PATH_UNIX}")
        target_link_libraries(${PROJECT_NAME} PUBLIC "${BOSON_LIB_PATH_UNIX}")
    else()
        message(WARNING "Boson library not found at ${BOSON_LIB_PATH_UNIX}")
    endif()
endif()

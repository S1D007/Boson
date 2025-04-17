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

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${BOSON_DIR}/include"
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC "${BOSON_DIR}/libboson.lib")
else()
    target_link_libraries(${PROJECT_NAME} "${BOSON_DIR}/libboson.a")
endif()
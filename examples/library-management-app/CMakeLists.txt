cmake_minimum_required(VERSION 3.10)
project(library-management-app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include all source files
add_executable(library-management-app 
    src/main.cpp
    models/book.cpp
    models/user.cpp
    models/loan.cpp
    controllers/book_controller.cpp
    controllers/user_controller.cpp
)

# Include directories
target_include_directories(library-management-app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link with the Boson library
target_link_libraries(library-management-app PUBLIC boson)
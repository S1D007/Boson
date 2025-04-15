set(SOURCES
    server.cpp
    router.cpp
    middleware.cpp
    request.cpp
    response.cpp
    controller.cpp
    error_handler.cpp
)

add_library(boson STATIC ${SOURCES})
target_include_directories(boson PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Add libcurl dependency for MongoDB adapter
find_package(CURL REQUIRED)
target_link_libraries(boson PUBLIC ${CURL_LIBRARIES})
target_include_directories(boson PUBLIC ${CURL_INCLUDE_DIRS})
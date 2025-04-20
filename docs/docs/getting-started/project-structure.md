---
sidebar_position: 3
title: Project Structure
---

# Boson Project Structure

A well-organized project structure helps maintain your code as your application grows. This guide suggests a recommended structure for Boson applications of different sizes.

## Basic Project Structure

For simple applications or APIs, this minimal structure works well:

```
my-boson-app/
├── CMakeLists.txt              # Project configuration
├── src/                        # Source code
│   ├── main.cpp                # Application entry point
│   ├── controllers/            # Route handlers grouped by resource
│   │   └── user_controller.hpp
│   ├── models/                 # Data structures and business objects
│   │   └── user.hpp
│   └── middleware/             # Custom middleware components
│       └── auth_middleware.hpp
├── config/                     # Configuration files
│   └── app.json
└── public/                     # Static files (if any)
    ├── css/
    ├── js/
    └── images/
```

## Directory Structure Explained

### src/

The `src` directory contains all your application's source code:

- **main.cpp**: Application entry point that sets up the server
- **controllers/**: Classes that handle related routes (e.g., UserController)
- **models/**: Data structures and business logic
- **middleware/**: Custom middleware for request/response processing

### Example Controller

```cpp
// src/controllers/user_controller.hpp
#pragma once
#include <boson/boson.hpp>
#include "../models/user.hpp"

class UserController {
public:
    // Get all users
    static void getUsers(const boson::Request& req, boson::Response& res) {
        // Implementation...
        res.jsonObject(/* users list */);
    }
    
    // Get user by ID
    static void getUserById(const boson::Request& req, boson::Response& res) {
        std::string id = req.param("id");
        // Implementation...
    }
    
    // Create a new user
    static void createUser(const boson::Request& req, boson::Response& res) {
        nlohmann::json body = req.json();
        // Implementation...
    }
    
    // Register all routes related to users
    static void registerRoutes(boson::Server& app) {
        app.get("/api/users", &UserController::getUsers);
        app.get("/api/users/:id", &UserController::getUserById);
        app.post("/api/users", &UserController::createUser);
    }
};
```

## Advanced Project Structure

For larger applications with multiple features, consider a more modular structure:

```
my-boson-app/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── app/                    # Application core
│   │   ├── config.hpp          # Configuration handling
│   │   └── errors.hpp          # Error handling utilities
│   ├── controllers/            # Organized by resource/feature
│   │   ├── auth_controller.hpp
│   │   ├── user_controller.hpp
│   │   └── product_controller.hpp
│   ├── models/                 # Data models
│   │   ├── user.hpp
│   │   └── product.hpp
│   ├── middleware/             # Middleware components
│   │   ├── auth_middleware.hpp
│   │   ├── logging_middleware.hpp
│   │   └── cors_middleware.hpp
│   └── services/               # Business logic services
│       ├── user_service.hpp
│       └── auth_service.hpp
├── include/                    # Public headers
│   └── my-boson-app/
├── tests/                      # Unit and integration tests
│   ├── controllers/
│   └── models/
├── config/                     # Configuration files
│   ├── development.json
│   └── production.json
└── public/                     # Static assets
```

## CMake Configuration Example

Here's a more comprehensive CMakeLists.txt for larger projects:

```cmake
cmake_minimum_required(VERSION 3.14)
project(my-boson-app VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boson package
find_package(Boson REQUIRED)

# Collect source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.hpp" "include/*.hpp")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link against Boson library
target_link_libraries(${PROJECT_NAME} PRIVATE Boson::Boson)

# Copy configuration and static files to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/public DESTINATION ${CMAKE_BINARY_DIR})
```

## Best Practices

1. **Separation of Concerns**: Keep controllers thin and move business logic to services
2. **Resource-Based Organization**: Group files by feature/resource rather than by type
3. **Consistent Naming**: Use consistent naming conventions (e.g., user_controller.hpp, UserController)
4. **Single Responsibility**: Each class should have a clear, single purpose
5. **Configuration Management**: Use environment-specific configuration files
6. **Error Handling**: Implement consistent error handling across the application

## Next Steps

With your project structure in place, you're ready to explore the core concepts of Boson:

- Learn about [Routing](../core-concepts/routing)
- Understand how to use [Controllers](../core-concepts/controllers)
- Explore [Middleware](../core-concepts/middleware) for cross-cutting concerns
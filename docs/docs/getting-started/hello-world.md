---
sidebar_position: 4
title: Hello World Tutorial
---

# Hello World in Boson

This tutorial provides a detailed walkthrough of creating a basic "Hello World" application using the Boson framework. We'll explore key concepts while building a simple web application that serves different types of responses.

## Project Overview

Our Hello World application will:
- Serve an HTML welcome page
- Provide a JSON endpoint for API testing
- Handle URL parameters
- Implement a simple middleware
- Demonstrate error handling

## Project Setup

### Step 1: Create the Project Directory

Start by creating a new directory for your project:

```bash
mkdir boson-hello-world
cd boson-hello-world
```

### Step 2: Create Project Structure

Set up the following project structure:

```
boson-hello-world/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── controllers/
│   │   └── hello_controller.hpp
│   └── middleware/
│       └── logging_middleware.hpp
└── public/
    └── index.html
```

### Step 3: Configure CMake

Create a `CMakeLists.txt` file with the following content:

```cmake
cmake_minimum_required(VERSION 3.14)
project(boson-hello-world VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boson REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE Boson::Boson)

# Copy static files to build directory
file(COPY ${CMAKE_SOURCE_DIR}/public DESTINATION ${CMAKE_BINARY_DIR})
```

## Creating Application Components

### Step 1: Creating a Simple HTML Page

Create a basic HTML page in `public/index.html`:

```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Boson Hello World</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            line-height: 1.6;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
        }
        h1 {
            color: #333;
            border-bottom: 1px solid #ddd;
            padding-bottom: 10px;
        }
        .container {
            background-color: #f9f9f9;
            border-radius: 5px;
            padding: 20px;
            margin-top: 20px;
        }
        code {
            background-color: #eee;
            padding: 2px 4px;
            border-radius: 3px;
            font-family: monospace;
        }
    </style>
</head>
<body>
    <h1>Hello from Boson Framework!</h1>
    
    <div class="container">
        <h2>Welcome to your first Boson application</h2>
        <p>This page is being served by the Boson C++ web framework.</p>
        <p>Try these endpoints:</p>
        <ul>
            <li><code>/</code> - This HTML page</li>
            <li><code>/hello</code> - A simple text response</li>
            <li><code>/hello/json</code> - A JSON response</li>
            <li><code>/hello/{name}</code> - A personalized greeting (replace {name} with your name)</li>
            <li><code>/error</code> - Demonstrates error handling</li>
        </ul>
    </div>
</body>
</html>
```

### Step 2: Creating a Custom Middleware

Create a simple logging middleware in `src/middleware/logging_middleware.hpp`:

```cpp
#pragma once

#include <boson/middleware.hpp>
#include <boson/request.hpp>
#include <boson/response.hpp>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <ctime>

class LoggingMiddleware : public boson::Middleware {
public:
    boson::Response process(const boson::Request& request, 
                           MiddlewareNext next) override {
        // Record the start time
        auto start = std::chrono::steady_clock::now();
        
        // Get current time for log
        auto now = std::time(nullptr);
        auto tm = std::localtime(&now);
        
        // Log the incoming request
        std::cout << "[" << std::put_time(tm, "%Y-%m-%d %H:%M:%S") << "] " 
                  << request.method() << " " 
                  << request.path() << std::endl;
        
        // Process the request through the rest of the middleware chain
        auto response = next(request);
        
        // Calculate request processing time
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        // Log the response
        std::cout << "[" << std::put_time(tm, "%Y-%m-%d %H:%M:%S") << "] " 
                  << "Responded with " << response.statusCode() << " in " 
                  << duration << "ms" << std::endl;
        
        return response;
    }
};
```

### Step 3: Creating a Controller

Create a controller to handle our routes in `src/controllers/hello_controller.hpp`:

```cpp
#pragma once

#include <boson/controller.hpp>
#include <boson/response.hpp>
#include <string>

class HelloController : public boson::Controller {
public:
    void registerRoutes() override {
        // Register routes with their handlers
        GET("/", &HelloController::index);
        GET("/hello", &HelloController::hello);
        GET("/hello/json", &HelloController::helloJson);
        GET("/hello/{name}", &HelloController::helloName);
        GET("/error", &HelloController::triggerError);
    }

    // Serve the static HTML page
    boson::Response index(const boson::Request& request) {
        return boson::Response::file("public/index.html", "text/html");
    }
    
    // Return a simple text response
    boson::Response hello(const boson::Request& request) {
        return boson::Response::ok("Hello, World!")
            .contentType("text/plain");
    }
    
    // Return a JSON response
    boson::Response helloJson(const boson::Request& request) {
        return boson::Response::json({
            {"message", "Hello, World!"},
            {"framework", "Boson"},
            {"language", "C++"},
            {"timestamp", std::time(nullptr)}
        });
    }
    
    // Handle a URL parameter
    boson::Response helloName(const boson::Request& request) {
        // Extract the name from the URL
        std::string name = request.param("name");
        
        // Return a personalized response
        return boson::Response::ok("Hello, " + name + "!")
            .contentType("text/plain");
    }
    
    // Demonstrate error handling
    boson::Response triggerError(const boson::Request& request) {
        // Randomly choose between different errors
        int errorType = std::rand() % 3;
        
        switch (errorType) {
            case 0:
                return boson::Response::notFound()
                    .json({{"error", "Resource not found"}});
            case 1:
                return boson::Response::serverError()
                    .json({{"error", "Internal server error"}});
            default:
                return boson::Response::forbidden()
                    .json({{"error", "Access forbidden"}});
        }
    }
};
```

### Step 4: Creating the Main Application

Create the main application file in `src/main.cpp`:

```cpp
#include <boson/boson.hpp>
#include <boson/server.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "controllers/hello_controller.hpp"
#include "middleware/logging_middleware.hpp"

int main() {
    try {
        // Seed random number generator for error demo
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        
        // Create the Boson application
        boson::Application app;
        
        // Register the global middleware
        app.useMiddleware<LoggingMiddleware>();
        
        // Register controllers
        app.registerController<HelloController>();
        
        // Create and configure the server
        boson::Server server(app);
        
        // Start the server
        std::cout << "Server starting at http://127.0.0.1:8080" << std::endl;
        server.listen("127.0.0.1", 8080);
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
```

## Building and Running the Application

### Step 1: Build the Application

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Step 2: Run the Application

```bash
./boson-hello-world
```

You should see output indicating that the server has started:

```
Server starting at http://127.0.0.1:8080
```

### Step 3: Test the Application

Open a web browser and navigate to:
- http://127.0.0.1:8080/ - The welcome page
- http://127.0.0.1:8080/hello - A simple text response
- http://127.0.0.1:8080/hello/json - A JSON response
- http://127.0.0.1:8080/hello/YourName - A personalized greeting
- http://127.0.0.1:8080/error - A random error response

Watch your terminal to see the logging middleware in action as you visit different URLs!

## Understanding the Code

### The Application Class

The `boson::Application` class is the central component that coordinates controllers, middleware, and other services. It's responsible for:
- Registering controllers and their routes
- Managing the middleware pipeline
- Handling application configuration
- Providing core services to the application

### The Controller Class

Controllers organize related route handlers together. The `HelloController` inherits from `boson::Controller` and:
- Defines route mappings in `registerRoutes()`
- Implements handler methods for different endpoints
- Processes requests and generates appropriate responses

### Middleware

Middleware provides a way to process requests and responses at different stages of the request lifecycle. Our `LoggingMiddleware`:
- Executes code before a request reaches route handlers
- Executes code after a response is generated
- Measures and logs request processing time

### Request and Response

The `boson::Request` class represents an incoming HTTP request, providing access to:
- URL parameters
- Query parameters
- HTTP headers
- Request body

The `boson::Response` class represents an HTTP response with methods for:
- Setting status codes
- Setting headers
- Setting content type
- Setting the response body
- Serving files

## Next Steps

Now that you've built a basic Boson application, you can:

1. Explore the [Project Structure](project-structure.md) guide to understand best practices
2. Learn about [Routing](../core-concepts/routing.md) in more depth

Congratulations on completing your first Boson application!
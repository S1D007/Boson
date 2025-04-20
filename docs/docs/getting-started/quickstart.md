---
sidebar_position: 2
title: Quickstart
---

# Boson Quickstart Guide

This quickstart guide will help you create a simple but functional Boson application in just a few minutes. You'll build a RESTful API with JSON responses.

## Step 1: Create a Project Structure

First, create a directory for your project with the following structure:

```
my-boson-app/
├── CMakeLists.txt
├── src/
│   └── main.cpp
```

## Step 2: Create CMakeLists.txt

Create a `CMakeLists.txt` file in your project root:

```cmake
cmake_minimum_required(VERSION 3.14)
project(my-boson-app VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boson package
find_package(Boson REQUIRED)

# Add executable
add_executable(${PROJECT_NAME} src/main.cpp)

# Link against Boson library
target_link_libraries(${PROJECT_NAME} PRIVATE Boson::Boson)
```

## Step 3: Create Your Application

In `src/main.cpp`, create a simple API server:

```cpp
#include <boson/boson.hpp>
#include <iostream>
#include <vector>
#include <string>

// A simple user model
struct User {
    int id;
    std::string name;
    std::string email;
    
    // Convert to JSON object using nlohmann::json library
    nlohmann::json toJson() const {
        return {
            {"id", id},
            {"name", name},
            {"email", email}
        };
    }
};

// Our in-memory "database"
std::vector<User> users = {
    {1, "John Doe", "john@example.com"},
    {2, "Jane Smith", "jane@example.com"}
};

int main() {
    // Initialize framework
    boson::initialize();
    
    // Create server instance
    boson::Server app;
    
    // Add logging middleware
    app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
        std::cout << "[" << req.method() << "] " << req.path() << std::endl;
        next();
    });
    
    // Define routes
    
    // GET /
    app.get("/", [](const boson::Request& req, boson::Response& res) {
        res.send("Welcome to the Boson API!");
    });
    
    // GET /api/users - Get all users
    app.get("/api/users", [](const boson::Request& req, boson::Response& res) {
        nlohmann::json usersJson = nlohmann::json::array();
        
        for (const auto& user : users) {
            usersJson.push_back(user.toJson());
        }
        
        nlohmann::json response = {
            {"users", usersJson},
            {"count", users.size()}
        };
        
        res.jsonObject(response);
    });
    
    // GET /api/users/:id - Get user by ID
    app.get("/api/users/:id", [](const boson::Request& req, boson::Response& res) {
        try {
            int id = std::stoi(req.param("id"));
            
            for (const auto& user : users) {
                if (user.id == id) {
                    res.jsonObject(user.toJson());
                    return;
                }
            }
            
            // User not found
            res.status(404).jsonObject({
                {"error", "User not found"},
                {"id", id}
            });
        }
        catch (const std::exception& e) {
            res.status(400).jsonObject({
                {"error", "Invalid ID format"},
                {"message", e.what()}
            });
        }
    });
    
    // Configure the server
    std::cout << "Starting server on http://127.0.0.1:3000" << std::endl;
    app.configure(3000, "127.0.0.1");
    
    // Start the server
    return app.listen();
}
```

## Step 4: Build Your Application

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
cmake --build .
```

## Step 5: Run Your Application

```bash
# Run from the build directory
./my-boson-app
```

Your server should start on port 3000. You can now test your API with curl:

```bash
# Get the welcome message
curl http://localhost:3000/

# Get all users
curl http://localhost:3000/api/users

# Get a specific user
curl http://localhost:3000/api/users/1
```

## Step 6: Extend Your API

Now that you have a working application, try extending it with:

- A POST endpoint to create new users
- A PUT endpoint to update users
- A DELETE endpoint to remove users

## Next Steps

Congratulations on creating your first Boson application! To learn more about organizing your code and building more complex applications:

- Check out the [Project Structure](project-structure) guide
- Explore [Core Concepts](../core-concepts/server) like routing and middleware
- See more [Examples](../examples/rest-api) for inspiration
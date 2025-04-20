---
sidebar_position: 2
title: Quickstart
---

# Boson Quickstart Guide

This quickstart guide will help you create a fully functional RESTful API with Boson in just a few minutes. By the end of this guide, you'll have a working API server with proper routing, JSON handling, and error management.

## Using the Boson CLI (Recommended)

The fastest way to get started is with the Boson CLI. If you've already [installed Boson](installation), you can create a new project with:

```bash
# Create a new project
boson new my-boson-api --template api

# Navigate to the project
cd my-boson-api

# Run the development server
boson run
```

Your server will start at http://localhost:3000, and you can begin testing your API immediately.

## Manual Project Setup

If you prefer setting up your project manually, follow these steps for complete control over your project structure.

### Step 1: Create a Project Structure

First, create a directory for your project with the following structure:

```
my-boson-app/
├── CMakeLists.txt
├── src/
│   └── main.cpp
```

You can create this structure with these commands:

```bash
mkdir -p my-boson-app/src
touch my-boson-app/CMakeLists.txt
touch my-boson-app/src/main.cpp
cd my-boson-app
```

### Step 2: Configure CMake

Create a `CMakeLists.txt` file in your project root with the following content:

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

This CMake configuration:
- Sets the C++ standard to C++17 (required by Boson)
- Finds the Boson package on your system
- Creates an executable from your source code
- Links it against the Boson library

### Step 3: Create Your Application

In `src/main.cpp`, create a RESTful API server with the following code:

```cpp
#include <boson/boson.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

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

// Find the next available user ID
int getNextUserId() {
    return users.empty() ? 1 : (std::max_element(users.begin(), users.end(), 
        [](const User& a, const User& b) { return a.id < b.id; })->id + 1);
}

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
    
    // GET / - Welcome message
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
            
            auto it = std::find_if(users.begin(), users.end(), 
                [id](const User& user) { return user.id == id; });
            
            if (it != users.end()) {
                res.jsonObject(it->toJson());
                return;
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
    
    // POST /api/users - Create a new user
    app.post("/api/users", [](const boson::Request& req, boson::Response& res) {
        try {
            // Parse request body as JSON
            nlohmann::json body = req.json();
            
            // Validate required fields
            if (!body.contains("name") || !body.contains("email")) {
                res.status(400).jsonObject({
                    {"error", "Missing required fields"},
                    {"required", {"name", "email"}}
                });
                return;
            }
            
            // Create new user
            User newUser;
            newUser.id = getNextUserId();
            newUser.name = body["name"].get<std::string>();
            newUser.email = body["email"].get<std::string>();
            
            // Add to our "database"
            users.push_back(newUser);
            
            // Return the created user
            res.status(201).jsonObject(newUser.toJson());
        }
        catch (const std::exception& e) {
            res.status(400).jsonObject({
                {"error", "Invalid request"},
                {"message", e.what()}
            });
        }
    });
    
    // PUT /api/users/:id - Update a user
    app.put("/api/users/:id", [](const boson::Request& req, boson::Response& res) {
        try {
            int id = std::stoi(req.param("id"));
            nlohmann::json body = req.json();
            
            auto it = std::find_if(users.begin(), users.end(), 
                [id](const User& user) { return user.id == id; });
            
            if (it != users.end()) {
                // Update user fields if they exist in the request
                if (body.contains("name")) {
                    it->name = body["name"].get<std::string>();
                }
                if (body.contains("email")) {
                    it->email = body["email"].get<std::string>();
                }
                
                res.jsonObject(it->toJson());
                return;
            }
            
            res.status(404).jsonObject({
                {"error", "User not found"},
                {"id", id}
            });
        }
        catch (const std::exception& e) {
            res.status(400).jsonObject({
                {"error", "Invalid request"},
                {"message", e.what()}
            });
        }
    });
    
    // DELETE /api/users/:id - Delete a user
    app.del("/api/users/:id", [](const boson::Request& req, boson::Response& res) {
        try {
            int id = std::stoi(req.param("id"));
            
            auto it = std::find_if(users.begin(), users.end(), 
                [id](const User& user) { return user.id == id; });
            
            if (it != users.end()) {
                users.erase(it);
                res.status(204).send("");
                return;
            }
            
            res.status(404).jsonObject({
                {"error", "User not found"},
                {"id", id}
            });
        }
        catch (const std::exception& e) {
            res.status(400).jsonObject({
                {"error", "Invalid request"},
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

This application implements:
- A simple User model with JSON serialization
- An in-memory "database" using a vector
- RESTful routes for CRUD operations
- Request logging middleware
- Error handling with appropriate HTTP status codes

### Step 4: Build Your Application

Now that you have your application code ready, build it with CMake:

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
cmake --build .
```

### Step 5: Run Your Application

Once built, you can run your application from the build directory:

```bash
# Run from the build directory
./my-boson-app
```

You should see output indicating that your server has started on port 3000.

## Testing Your API

You can test your RESTful API using curl, Postman, or any HTTP client. Here are some curl examples:

### Get All Users

```bash
curl http://localhost:3000/api/users
```

Expected response:
```json
{
  "count": 2,
  "users": [
    {
      "email": "john@example.com",
      "id": 1,
      "name": "John Doe"
    },
    {
      "email": "jane@example.com",
      "id": 2,
      "name": "Jane Smith"
    }
  ]
}
```

### Get a Specific User

```bash
curl http://localhost:3000/api/users/1
```

Expected response:
```json
{
  "email": "john@example.com",
  "id": 1,
  "name": "John Doe"
}
```

### Create a New User

```bash
curl -X POST \
  http://localhost:3000/api/users \
  -H 'Content-Type: application/json' \
  -d '{"name":"Bob Johnson","email":"bob@example.com"}'
```

Expected response:
```json
{
  "email": "bob@example.com",
  "id": 3,
  "name": "Bob Johnson"
}
```

### Update a User

```bash
curl -X PUT \
  http://localhost:3000/api/users/2 \
  -H 'Content-Type: application/json' \
  -d '{"name":"Jane Williams"}'
```

Expected response:
```json
{
  "email": "jane@example.com",
  "id": 2,
  "name": "Jane Williams"
}
```

### Delete a User

```bash
curl -X DELETE http://localhost:3000/api/users/3
```

This should return a 204 No Content response with no body.

## Code Breakdown

Let's examine some key parts of the application:

### Server Initialization

```cpp
// Initialize framework
boson::initialize();

// Create server instance
boson::Server app;
```

These lines initialize the Boson framework and create a server instance. The `initialize()` function sets up internal components.

### Middleware

```cpp
app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    std::cout << "[" << req.method() << "] " << req.path() << std::endl;
    next();
});
```

This adds a simple logging middleware that prints the HTTP method and path for each request. The `next()` function continues processing to the next middleware or route handler.

### Route Handling

```cpp
app.get("/api/users/:id", [](const boson::Request& req, boson::Response& res) {
    // Handler code
});
```

This defines a route that responds to GET requests at the path `/api/users/:id`, where `:id` is a path parameter that can be accessed with `req.param("id")`.

### JSON Handling

```cpp
nlohmann::json response = {
    {"users", usersJson},
    {"count", users.size()}
};

res.jsonObject(response);
```

Boson integrates with the `nlohmann::json` library to make working with JSON straightforward. The `jsonObject()` method sets the appropriate Content-Type header and serializes the JSON object.

### Error Handling

```cpp
try {
    // Code that might throw
}
catch (const std::exception& e) {
    res.status(400).jsonObject({
        {"error", "Invalid request"},
        {"message", e.what()}
    });
}
```

Proper error handling ensures that the client receives meaningful error messages with appropriate HTTP status codes.

## Next Steps

Congratulations on creating your first Boson application! You now have a fully functional RESTful API with proper error handling and JSON support. To continue your Boson journey:

- Explore the [Project Structure](project-structure) guide to learn how to organize larger applications
- Learn about [Middleware](../core-concepts/middleware) to add functionality like authentication, CORS, etc.
- Read the [Server](../core-concepts/server) documentation to understand configuration options
- Check out [Request and Response](../core-concepts/request-response) for more advanced handling techniques
- See how to use [Controllers](../core-concepts/controllers) to organize your route handlers
- Browse the [Examples](../examples/rest-api) for more inspiration

Ready to build something more complex? Continue to the [Hello World Tutorial](hello-world) for a more detailed walkthrough of building a multi-feature application.
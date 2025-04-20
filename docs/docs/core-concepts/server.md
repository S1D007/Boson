---
sidebar_position: 1
title: Server
---

# Boson Server

The Boson Server is the central component that powers your web application. It handles HTTP requests, manages connections, routes incoming traffic, and serves as the foundation for your entire application.

## Server Overview

The `boson::Server` class provides:

- HTTP request handling for various methods (GET, POST, PUT, DELETE, etc.)
- Middleware support for request/response processing
- Routing capabilities with path parameters
- Error handling mechanisms
- Configuration options for ports, hosts, and other settings
- Performance optimizations for handling concurrent requests

## Creating a Server

Creating a Boson server involves these basic steps:

```cpp
#include <boson/boson.hpp>

int main() {
    // Initialize the framework
    boson::initialize();
    
    // Create a server instance
    boson::Server app;
    
    // Define routes and middleware...
    
    // Configure and start the server
    app.configure(3000, "127.0.0.1");
    return app.listen();
}
```

The call to `boson::initialize()` is required before using any Boson functionality, as it sets up internal components and prepares the environment.

## Server Configuration

The `configure` method allows you to set the port and host address for your server:

```cpp
// Listen on port 3000 on all interfaces
app.configure(3000, "0.0.0.0");

// Listen on port 8080 on localhost only
app.configure(8080, "127.0.0.1");

// Listen on port 443 for HTTPS (requires SSL configuration)
app.configure(443, "0.0.0.0");
```

### Advanced Configuration

For more advanced configuration, you can use additional settings:

```cpp
// Configure with timeout settings
app.setTimeout(30); // 30-second timeout for requests

// Configure maximum concurrent connections
app.setMaxConnections(1000);

// Configure keep-alive settings
app.setKeepAliveTimeout(60); // 60-second keep-alive timeout

// Configure SSL/TLS (HTTPS)
app.enableSSL("path/to/cert.pem", "path/to/key.pem");
```

## HTTP Request Handlers

Boson supports all standard HTTP methods. Each handler receives the request and response objects:

### GET Requests

```cpp
app.get("/hello", [](const boson::Request& req, boson::Response& res) {
    res.send("Hello, World!");
});

// With path parameters
app.get("/users/:id", [](const boson::Request& req, boson::Response& res) {
    std::string id = req.param("id");
    res.send("User ID: " + id);
});

// With query parameters
app.get("/search", [](const boson::Request& req, boson::Response& res) {
    std::string query = req.query("q");
    res.send("Search query: " + query);
});
```

### POST Requests

```cpp
app.post("/users", [](const boson::Request& req, boson::Response& res) {
    // Parse JSON request body
    auto data = req.json();
    
    // Extract and validate data
    if (!data.contains("name") || !data.contains("email")) {
        res.status(400).jsonObject({{"error", "Missing required fields"}});
        return;
    }
    
    // Process data (in a real app, you might save to a database)
    std::string name = data["name"].get<std::string>();
    std::string email = data["email"].get<std::string>();
    
    // Respond with success
    res.status(201).jsonObject({
        {"message", "User created"},
        {"user", {{"name", name}, {"email", email}}}
    });
});
```

### PUT Requests

```cpp
app.put("/users/:id", [](const boson::Request& req, boson::Response& res) {
    std::string id = req.param("id");
    auto data = req.json();
    
    // Update user (in a real app, you might update a database)
    
    res.jsonObject({
        {"message", "User updated"},
        {"id", id}
    });
});
```

### DELETE Requests

```cpp
app.del("/users/:id", [](const boson::Request& req, boson::Response& res) {
    std::string id = req.param("id");
    
    // Delete user (in a real app, you might delete from a database)
    
    res.status(204).send("");  // 204 No Content is common for successful DELETE
});
```

### PATCH Requests

```cpp
app.patch("/users/:id", [](const boson::Request& req, boson::Response& res) {
    std::string id = req.param("id");
    auto data = req.json();
    
    // Partially update user
    
    res.jsonObject({
        {"message", "User partially updated"},
        {"id", id}
    });
});
```

### OPTIONS Requests

```cpp
app.options("/api/:resource", [](const boson::Request& req, boson::Response& res) {
    // Handle CORS preflight requests
    res.headers({
        {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE"},
        {"Access-Control-Allow-Headers", "Content-Type, Authorization"},
        {"Access-Control-Max-Age", "86400"}  // 24 hours
    });
    res.status(204).send("");
});
```

### ALL Method (Any HTTP Method)

```cpp
app.all("/api/:resource", [](const boson::Request& req, boson::Response& res) {
    // This handler will match any HTTP method for the path
    res.send("Handled request with method: " + req.method());
});
```

## Adding Middleware

Middleware functions process requests before they reach route handlers. They can modify the request/response objects, end the response early, or pass control to the next middleware.

### Global Middleware

Global middleware is applied to all routes:

```cpp
// Request logging middleware
app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    std::cout << "[" << req.method() << "] " << req.path() << std::endl;
    
    // Call next to continue to the next middleware or route handler
    next();
});

// Request timing middleware
app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Store the start time in the request for later access
    req.set("requestStart", start);
    
    // Continue processing
    next();
    
    // After response is prepared
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "Request processed in " << duration << "ms" << std::endl;
});
```

### Path-Specific Middleware

Middleware can be applied to specific paths:

```cpp
// Apply middleware only to /api routes
app.use("/api", [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    std::cout << "API request: " << req.path() << std::endl;
    next();
});

// Authentication middleware for protected routes
app.use("/admin", [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    std::string token = req.header("Authorization");
    
    if (token.empty() || !validateToken(token)) {
        res.status(401).jsonObject({{"error", "Unauthorized"}});
        // Not calling next() ends the request here
        return;
    }
    
    // Token is valid, continue to the next middleware or route handler
    next();
});
```

### Middleware Order

Middleware is processed in the order it's added. This is important to consider when designing your application:

```cpp
// This logging middleware will run first
app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    std::cout << "First middleware" << std::endl;
    next();
});

// This middleware will run second
app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    std::cout << "Second middleware" << std::endl;
    next();
});

// Route handlers run after all matching middleware
app.get("/", [](const boson::Request& req, boson::Response& res) {
    res.send("Hello, World!");
});
```

## Using Routers

For larger applications, you can organize routes using the Router class:

```cpp
// Create a router for user-related routes
boson::Router userRouter;

// Define routes on the router
userRouter.get("/", [](const boson::Request& req, boson::Response& res) {
    res.send("Get all users");
});

userRouter.get("/:id", [](const boson::Request& req, boson::Response& res) {
    res.send("Get user " + req.param("id"));
});

userRouter.post("/", [](const boson::Request& req, boson::Response& res) {
    res.send("Create user");
});

// Mount the router on the main app at /api/users
app.use("/api/users", userRouter);
```

## Error Handling

Boson provides several ways to handle errors in your application.

### Try-Catch in Middleware

You can wrap route handlers in a try-catch block using middleware:

```cpp
// Error handling middleware
app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    try {
        next();
    } catch (const std::exception& e) {
        res.status(500).jsonObject({
            {"error", "Internal Server Error"},
            {"message", e.what()},
            {"path", req.path()}
        });
    }
});
```

### Global Error Handler

You can set a global error handler for uncaught exceptions:

```cpp
app.setErrorHandler([](const std::exception& e, const boson::Request& req, boson::Response& res) {
    // Log the error
    std::cerr << "Error: " << e.what() << " in " << req.path() << std::endl;
    
    // Send an appropriate response
    res.status(500).jsonObject({
        {"error", "Internal Server Error"},
        {"message", "An unexpected error occurred"}
    });
});
```

### Route-Specific Error Handling

Handle errors within specific routes:

```cpp
app.get("/api/data", [](const boson::Request& req, boson::Response& res) {
    try {
        // Code that might throw
        throw std::runtime_error("Something went wrong");
    } catch (const std::exception& e) {
        res.status(500).jsonObject({
            {"error", "Error fetching data"},
            {"message", e.what()}
        });
    }
});
```

## Graceful Shutdown

Handling graceful shutdown ensures connections are properly closed:

```cpp
#include <csignal>

boson::Server app;

// Signal handler function
void signalHandler(int signal) {
    std::cout << "Received signal " << signal << std::endl;
    std::cout << "Shutting down server gracefully..." << std::endl;
    app.stop();  // Stop accepting new connections and close existing ones properly
}

int main() {
    // Register signal handlers
    std::signal(SIGINT, signalHandler);   // Handle Ctrl+C
    std::signal(SIGTERM, signalHandler);  // Handle termination signal
    
    boson::initialize();
    
    // Configure and start the server
    app.configure(3000, "127.0.0.1");
    std::cout << "Server running at http://127.0.0.1:3000" << std::endl;
    
    return app.listen();  // Start listening for requests
}
```

## Serving Static Files

Boson makes it easy to serve static files:

```cpp
// Serve static files from the "public" directory
app.use(boson::StaticFiles::create("./public", "/"));

// Serve static files with custom options
std::unordered_map<std::string, std::string> options = {
    {"cacheControl", "max-age=86400"},  // Cache for 24 hours
    {"etag", "true"}                    // Enable ETag headers
};
app.use(boson::StaticFiles::create("./assets", "/assets", options));
```

## Performance Considerations

Here are some tips for optimizing your Boson server:

- **Use release builds** for production for maximum performance
- **Minimize middleware** usage on hot paths
- **Consider the order** of your middleware (put frequently skipped middleware first)
- **Enable response compression** for text-based responses
- **Use appropriate status codes** and methods for proper HTTP caching
- **Implement pagination** for endpoints that return large datasets
- **Handle errors gracefully** to prevent server crashes

## Complete Server Example

Here's a complete example of a Boson server with multiple routes, middleware, and error handling:

```cpp
#include <boson/boson.hpp>
#include <iostream>
#include <chrono>
#include <csignal>

// Global server instance for signal handling
boson::Server app;

// Signal handler
void signalHandler(int signal) {
    std::cout << "Shutting down server..." << std::endl;
    app.stop();
}

int main() {
    // Register signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    try {
        // Initialize Boson
        boson::initialize();
        
        // Request logging middleware
        app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            std::cout << "[" << std::ctime(&time) << "] " 
                     << req.method() << " " << req.path() << std::endl;
            next();
        });
        
        // CORS middleware
        app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
            res.header("Access-Control-Allow-Origin", "*");
            res.header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");
            res.header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            
            if (req.method() == "OPTIONS") {
                res.status(204).send("");
                return;
            }
            
            next();
        });
        
        // Error handling middleware
        app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
            try {
                next();
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                res.status(500).jsonObject({
                    {"error", "Internal Server Error"},
                    {"message", e.what()}
                });
            }
        });
        
        // Serve static files
        app.use(boson::StaticFiles::create("./public", "/"));
        
        // Define API routes
        app.get("/", [](const boson::Request& req, boson::Response& res) {
            res.jsonObject({
                {"message", "Welcome to the Boson API"},
                {"version", "1.0"}
            });
        });
        
        app.get("/api/status", [](const boson::Request& req, boson::Response& res) {
            res.jsonObject({
                {"status", "operational"},
                {"time", std::time(nullptr)},
                {"uptime", "1h 23m"}  // In a real app, calculate actual uptime
            });
        });
        
        // Set up and start the server
        std::cout << "Starting server on http://127.0.0.1:3000" << std::endl;
        app.configure(3000, "127.0.0.1");
        return app.listen();
    }
    catch (const std::exception& e) {
        std::cerr << "Server initialization failed: " << e.what() << std::endl;
        return 1;
    }
}
```

## Conclusion

The Boson Server is a versatile and powerful component that forms the foundation of your web application. By understanding its capabilities and using them effectively, you can build robust, high-performance web applications and APIs.

In the next sections, you'll learn more about routing, middleware, controllers, and other core concepts that build on the server's functionality.
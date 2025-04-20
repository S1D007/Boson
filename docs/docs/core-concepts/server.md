---
sidebar_position: 1
title: Server
---

# Boson Server

The Boson server is the core component that powers your web application. It handles HTTP requests, manages connections, and provides the infrastructure for your routes and middleware.

## Creating a Server

Creating a Boson server is straightforward:

```cpp
#include <boson/boson.hpp>

int main() {

    boson::initialize();
    

    boson::Server app;
    

    app.configure(3000, "127.0.0.1");
    return app.listen();
}
```

## Server Configuration

The server can be configured with various options:

```cpp

app.configure(3000, "0.0.0.0");  

```

## HTTP Request Handlers

You can add request handlers for different HTTP methods:

```cpp

app.get("/hello", [](const boson::Request& req, boson::Response& res) {
    res.send("Hello, World!");
});


app.post("/users", [](const boson::Request& req, boson::Response& res) {
    auto data = req.json();

    res.status(201).send("User created");
});


app.put("/users/:id", [](const boson::Request& req, boson::Response& res) {
    std::string id = req.param("id");

    res.send("User updated");
});


app.del("/users/:id", [](const boson::Request& req, boson::Response& res) {
    std::string id = req.param("id");

    res.send("User deleted");
});
```

## Adding Middleware

Middleware functions process requests before they reach your route handlers:

```cpp

app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    std::cout << "[" << req.method() << "] " << req.path() << std::endl;
    next();  });


app.use("/api", [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {

    std::cout << "API request: " << req.path() << std::endl;
    next();
});
```

## Error Handling

Boson provides built-in error handling capabilities:

```cpp

app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    try {
        next();
    } catch (const std::exception& e) {
        res.status(500).jsonObject({
            {"error", e.what()},
            {"path", req.path()}
        });
    }
});
```

## Graceful Shutdown

To handle graceful shutdown of the server:

```cpp
#include <signal.h>

boson::Server app;

void signalHandler(int signal) {
    std::cout << "Shutting down server..." << std::endl;
    app.shutdown();  }

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    

    
    return app.listen();
}
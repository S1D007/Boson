---
sidebar_position: 1
title: Introduction
slug: /
---

# Introduction to Boson Framework

Boson is a modern, high-performance C++ web framework designed for building robust web applications and APIs. It combines the raw power and efficiency of C++ with an elegant, intuitive developer experience inspired by popular web frameworks like Express.js.

## What is Boson?

Boson is a C++17 web framework that provides:

- **High Performance**: Built from the ground up for speed with minimal overhead, making it ideal for high-traffic applications
- **Simple API**: Intuitive interfaces similar to popular web frameworks that make web development in C++ accessible
- **Modern C++ Design**: Leverages C++17 features for safer, cleaner code with RAII principles and smart pointers
- **Middleware Architecture**: Flexible middleware system for request processing pipelines
- **Flexibility**: Works for everything from small microservices to complex, full-featured applications

## Key Features

- **Fast HTTP Server**: Built from the ground up for performance, handling thousands of requests per second
- **Expressive Routing**: Simple definition of application endpoints with support for path parameters and patterns
- **Middleware Support**: Easily extend request/response processing with global or route-specific middleware
- **JSON Processing**: First-class support for JSON requests and responses using the integrated nlohmann::json library
- **Controller Architecture**: Organize related routes logically using controller classes
- **Error Handling**: Comprehensive error management with customizable error handlers
- **Static File Serving**: Efficiently serve static assets with configurable caching policies
- **Dependency-Free Core**: Minimal external dependencies for easy integration into any project

## Who Should Use Boson?

Boson is ideal for:

- Developers building high-performance web services where throughput and latency matter
- Teams that need the efficiency and control of C++ with a modern, ergonomic API
- Projects where low latency and resource usage are critical (finance, gaming, IoT)
- Existing C++ codebases that need to expose web interfaces
- Applications with specific performance requirements that can't be met by higher-level languages

## When to Consider Alternatives

Boson might not be the best fit when:

- Your team has no C++ experience and development speed is the primary concern
- You're building a simple CRUD application with no special performance requirements
- You need extensive ecosystem support and a vast library of plug-and-play modules

## Quick Example

Here's a simple "Hello World" example in Boson:

```cpp
#include <boson/boson.hpp>

int main() {
    // Initialize the framework
    boson::initialize();
    
    // Create a server
    boson::Server app;
    
    // Define a route
    app.get("/", [](const boson::Request& req, boson::Response& res) {
        res.send("Hello, Boson!");
    });
    
    // JSON response example
    app.get("/api/data", [](const boson::Request& req, boson::Response& res) {
        res.jsonObject({
            {"message", "Welcome to Boson"},
            {"version", "1.0"},
            {"success", true}
        });
    });
    
    // Configure and start the server
    app.configure(3000, "127.0.0.1");
    return app.listen();
}
```

## Philosophy

Boson is built on these core principles:

1. **Performance without compromise**: Optimized for speed without sacrificing developer experience
2. **Simplicity over complexity**: Clean APIs that make common tasks easy and complex tasks possible
3. **Safety by default**: Design patterns that promote memory safety and prevent common errors
4. **Flexibility with guidance**: Clear conventions with the flexibility to adapt to various use cases

Ready to get started? Head to the [Installation Guide](getting-started/installation) to begin your journey with Boson.

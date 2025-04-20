---
sidebar_position: 1
title: Introduction
slug: /
---

# Introduction to Boson Framework

Boson is a modern, high-performance C++ web framework designed for building robust web applications and APIs. It combines the power and efficiency of C++ with an elegant and intuitive developer experience.

## What is Boson?

Boson is a C++17 web framework that provides:

- **High Performance**: Built for speed with minimal overhead
- **Simple API**: Intuitive interfaces similar to popular web frameworks
- **Modern C++ Design**: Leverages C++17 features for safer, cleaner code
- **Flexibility**: Works for everything from small services to complex applications

## Key Features

- **Fast HTTP Server**: Built from the ground up for performance
- **Expressive Routing**: Simple definition of application endpoints
- **Middleware Support**: Easily extend request/response processing
- **JSON Processing**: First-class support for JSON requests and responses
- **Controller Architecture**: Organize related routes logically
- **Error Handling**: Comprehensive error management

## Who Should Use Boson?

Boson is ideal for:

- Developers building high-performance web services
- Teams that need the efficiency of C++ with a modern API
- Projects where low latency and resource usage are critical
- Existing C++ codebases that need web capabilities

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
    
    // Configure and start the server
    app.configure(3000, "127.0.0.1");
    return app.listen();
}
```

Ready to get started? Head to the [Installation Guide](getting-started/installation) to begin your journey with Boson.

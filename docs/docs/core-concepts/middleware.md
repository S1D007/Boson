---
sidebar_position: 3
title: Middleware
---

# Middleware in Boson

Middleware functions are a powerful way to process HTTP requests and responses in Boson. They enable you to execute code before a request reaches a route handler or after a response is generated.

## What is Middleware?

Middleware functions have access to the request object, the response object, and a `next` function that passes control to the next middleware in the chain. They can:

- Execute any code
- Modify request and response objects
- End the request-response cycle
- Call the next middleware in the chain

## Basic Middleware Usage

Adding middleware to your application is straightforward:

```cpp
// Create a server instance
boson::Server app;

// Add a simple logging middleware
app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    std::cout << "[" << req.method() << "] " << req.path() << std::endl;
    next();  // Call next middleware or route handler
});

// Add a route handler (not middleware)
app.get("/hello", [](const boson::Request& req, boson::Response& res) {
    res.send("Hello, World!");
});
```

## The Middleware Chain

Middleware functions are executed in the order they are added to the application. Each middleware must either:

1. Call `next()` to pass control to the next middleware
2. End the request by sending a response (e.g., `res.send()` or `res.jsonObject()`)

```cpp
// First middleware - always executed
app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    std::cout << "Middleware 1: Processing request" << std::endl;
    next();  // Continue to next middleware
});

// Second middleware - also always executed because previous middleware called next()
app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    std::cout << "Middleware 2: Processing request" << std::endl;
    next();  // Continue to next middleware
});

// Third middleware - ends the request for a specific path
app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    if (req.path() == "/blocked") {
        res.status(403).send("Access blocked");
        // Does not call next(), so the chain stops here for "/blocked"
    } else {
        next();  // Continue for other paths
    }
});

// This route handler is never reached for "/blocked" because the middleware ended the request
app.get("/blocked", [](const boson::Request& req, boson::Response& res) {
    res.send("You should never see this!");
});
```

## Path-Specific Middleware

You can apply middleware to specific paths:

```cpp
// This middleware only runs for requests to paths starting with "/api"
app.use("/api", [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    std::cout << "API request: " << req.path() << std::endl;
    next();
});

// This middleware runs for all requests
app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    std::cout << "Global middleware for all paths" << std::endl;
    next();
});
```

## Common Middleware Examples

### Authentication Middleware

```cpp
auto authenticate = [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    // Get the authorization header
    std::string token = req.header("Authorization");
    
    if (token.empty() || !token.starts_with("Bearer ")) {
        res.status(401).jsonObject({
            {"error", "Unauthorized"},
            {"message", "Authentication required"}
        });
        return;  // Stop the middleware chain
    }
    
    // Validate the token (simplified example)
    std::string actualToken = token.substr(7);  // Remove "Bearer " prefix
    
    if (actualToken != "valid-token") {
        res.status(403).jsonObject({
            {"error", "Forbidden"},
            {"message", "Invalid token"}
        });
        return;  // Stop the middleware chain
    }
    
    // Authentication successful, continue to next middleware or route handler
    next();
};

// Apply authentication to all API routes
app.use("/api", authenticate);

// Or apply it to a specific route
app.get("/admin/dashboard", authenticate, [](const boson::Request& req, boson::Response& res) {
    res.send("Admin dashboard");
});
```

### CORS Middleware

```cpp
auto corsMiddleware = [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    // Allow requests from any origin
    res.header("Access-Control-Allow-Origin", "*");
    
    // Allow specific headers
    res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept, Authorization");
    
    // Allow specific methods
    res.header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    
    // Handle preflight requests
    if (req.method() == "OPTIONS") {
        res.status(200).send();
        return;  // End the request here for OPTIONS
    }
    
    next();  // Continue for non-OPTIONS requests
};

// Apply CORS middleware to all routes
app.use(corsMiddleware);
```

### Request Logging Middleware

```cpp
auto requestLogger = [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Log request details
    std::cout << "[" << req.method() << "] " << req.path();
    
    // Store the original send method
    auto originalSend = res.getSendCallback();
    
    // Override the send method to log response details
    res.setSendCallback([originalSend, startTime, &req](boson::Response& r) {
        // Calculate request duration
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        // Log response details
        std::cout << " - " << r.statusCode() << " (" << duration.count() << "ms)" << std::endl;
        
        // Call the original send method
        originalSend(r);
    });
    
    next();
};

// Apply logging middleware to all routes
app.use(requestLogger);
```

### Error Handling Middleware

Error handling middleware should be defined at the end of the middleware chain:

```cpp
// Regular middleware functions
app.use(/* other middleware */);

// Define routes
app.get("/", /* route handler */);

// Error handling middleware (defined last)
app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    try {
        // Continue to the next middleware or route handler
        next();
    } catch (const boson::HttpError& e) {
        // Handle HTTP errors
        res.status(e.statusCode()).jsonObject({
            {"error", e.what()},
            {"status", e.statusCode()}
        });
    } catch (const std::exception& e) {
        // Handle other exceptions
        res.status(500).jsonObject({
            {"error", "Internal Server Error"},
            {"message", e.what()}
        });
    } catch (...) {
        // Handle unknown errors
        res.status(500).jsonObject({
            {"error", "Internal Server Error"},
            {"message", "An unknown error occurred"}
        });
    }
});
```

## Creating Reusable Middleware

You can create reusable middleware functions by defining them separately:

```cpp
// Define a reusable middleware function
boson::MiddlewareFunction requireAuth(const std::vector<std::string>& roles = {}) {
    return [roles](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
        // Get the authorization header
        std::string token = req.header("Authorization");
        
        // Basic validation (simplified example)
        if (token.empty()) {
            res.status(401).jsonObject({
                {"error", "Unauthorized"},
                {"message", "Authentication required"}
            });
            return;
        }
        
        // Check roles if specified
        if (!roles.empty()) {
            // Get user roles from token (simplified example)
            std::vector<std::string> userRoles = {"user"};  // In reality, extracted from the token
            
            bool hasRequiredRole = false;
            for (const auto& role : roles) {
                if (std::find(userRoles.begin(), userRoles.end(), role) != userRoles.end()) {
                    hasRequiredRole = true;
                    break;
                }
            }
            
            if (!hasRequiredRole) {
                res.status(403).jsonObject({
                    {"error", "Forbidden"},
                    {"message", "Insufficient permissions"}
                });
                return;
            }
        }
        
        next();
    };
}

// Use the middleware
app.get("/user/profile", requireAuth(), [](const boson::Request& req, boson::Response& res) {
    res.send("User profile");
});

app.get("/admin/dashboard", requireAuth({"admin"}), [](const boson::Request& req, boson::Response& res) {
    res.send("Admin dashboard");
});
```

## Middleware Best Practices

1. **Keep Middleware Focused**: Each middleware should have a single responsibility
2. **Order Matters**: Add middleware in the right order (e.g., logging before authentication)
3. **Don't Forget `next()`**: Always call `next()` unless you want to end the request
4. **Error Handling**: Use try-catch blocks to handle errors in middleware
5. **Reuse Middleware**: Define reusable middleware functions
6. **Test Middleware**: Write tests for your middleware functions
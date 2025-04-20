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

## Advanced Middleware Patterns

### Data Processing Pipeline

Middleware can be used to build processing pipelines for request data:

```cpp
// Middleware to parse and validate JSON body
auto parseJsonBody = [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    if (req.header("Content-Type").find("application/json") == std::string::npos) {
        next();
        return;
    }
    
    try {
        // Parse JSON (already built into Boson request)
        auto body = req.json();
        
        // Store the parsed body in the request for later middleware
        req.set("parsedBody", body);
        next();
    } catch (const std::exception& e) {
        res.status(400).jsonObject({
            {"error", "Invalid JSON"},
            {"message", e.what()}
        });
    }
};

// Middleware to validate user input schema
auto validateUserSchema = [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    if (!req.has("parsedBody")) {
        next();
        return;
    }
    
    auto body = req.get<nlohmann::json>("parsedBody");
    std::vector<std::string> missingFields;
    
    // Check required fields
    for (const auto& field : {"name", "email", "password"}) {
        if (!body.contains(field) || body[field].empty()) {
            missingFields.push_back(field);
        }
    }
    
    if (!missingFields.empty()) {
        nlohmann::json errorResponse = {{"error", "Validation failed"}};
        errorResponse["missing_fields"] = missingFields;
        res.status(400).jsonObject(errorResponse);
        return;
    }
    
    // Additional validation logic
    auto email = body["email"].get<std::string>();
    if (email.find("@") == std::string::npos) {
        res.status(400).jsonObject({
            {"error", "Invalid email format"}
        });
        return;
    }
    
    next();
};

// Register the middleware chain
app.post("/users", parseJsonBody, validateUserSchema, [](const boson::Request& req, boson::Response& res) {
    // By now we have valid JSON with all required fields
    auto userData = req.get<nlohmann::json>("parsedBody");
    
    // Process the validated user data
    res.status(201).jsonObject({
        {"message", "User created successfully"},
        {"user", {
            {"name", userData["name"]},
            {"email", userData["email"]}
        }}
    });
});
```

### Conditional Middleware

Execute middleware conditionally based on request properties:

```cpp
// Factory function to create conditional middleware
auto when = [](std::function<bool(const boson::Request&)> condition, boson::MiddlewareFunction middleware) {
    return [condition, middleware](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
        if (condition(req)) {
            middleware(req, res, next);
        } else {
            next();
        }
    };
};

// Example conditions
auto isApiRequest = [](const boson::Request& req) {
    return req.path().starts_with("/api");
};

auto isAuthenticatedRequest = [](const boson::Request& req) {
    return !req.header("Authorization").empty();
};

// Conditional middleware usage
app.use(when(isApiRequest, corsMiddleware));
app.use(when(isAuthenticatedRequest, userActivityLogger));
```

### Middleware for Specific HTTP Methods

Apply middleware only for specific HTTP methods:

```cpp
// Method-specific middleware
auto onlyForPost = [](boson::MiddlewareFunction middleware) {
    return [middleware](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
        if (req.method() == "POST") {
            middleware(req, res, next);
        } else {
            next();
        }
    };
};

// Apply middleware only to POST requests
app.use(onlyForPost(parseJsonBody));
```

### Request Context Sharing

Share data between middleware and route handlers:

```cpp
// First middleware populates context
app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    // Create a request-scoped context
    req.set("context", std::make_shared<std::unordered_map<std::string, std::any>>());
    
    // Set some data
    auto context = req.get<std::shared_ptr<std::unordered_map<std::string, std::any>>>("context");
    (*context)["requestTime"] = std::chrono::system_clock::now();
    (*context)["clientIp"] = req.ip();
    
    next();
});

// Later middleware can access and modify the context
app.use("/api", [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    auto context = req.get<std::shared_ptr<std::unordered_map<std::string, std::any>>>("context");
    (*context)["isApiRequest"] = true;
    
    next();
});

// Route handler can access the context
app.get("/api/status", [](const boson::Request& req, boson::Response& res) {
    auto context = req.get<std::shared_ptr<std::unordered_map<std::string, std::any>>>("context");
    
    res.jsonObject({
        {"status", "ok"},
        {"isApi", std::any_cast<bool>((*context)["isApiRequest"])},
        {"ip", std::any_cast<std::string>((*context)["clientIp"])}
    });
});
```

### Async Middleware

Handle asynchronous operations in middleware:

```cpp
// Async middleware pattern using std::future and promises
auto asyncMiddleware = [](std::function<void(const boson::Request&, boson::Response&, 
                                          std::function<void()>, std::function<void(std::string)>)> operation) {
    return [operation](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
        // Success callback
        auto success = [&next]() {
            next();
        };
        
        // Error callback
        auto error = [&res](const std::string& message) {
            res.status(500).jsonObject({{"error", message}});
        };
        
        // Execute the async operation
        operation(req, res, success, error);
    };
};

// Example usage with a database query
auto fetchUserFromDb = asyncMiddleware([](const boson::Request& req, boson::Response& res, 
                                      std::function<void()> success, std::function<void(std::string)> error) {
    std::string userId = req.param("id");
    
    // Simulate async database query
    std::thread([userId, &req, success, error]() {
        try {
            // Simulated DB query
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Simulate found/not found
            if (userId == "123") {
                // User found, attach to request
                req.set("user", nlohmann::json({
                    {"id", "123"},
                    {"name", "John Doe"},
                    {"email", "john@example.com"}
                }));
                success();
            } else {
                error("User not found");
            }
        } catch (const std::exception& e) {
            error(e.what());
        }
    }).detach();
});

// Use the async middleware
app.get("/users/:id", fetchUserFromDb, [](const boson::Request& req, boson::Response& res) {
    // Now we can safely access the user data
    auto user = req.get<nlohmann::json>("user");
    res.jsonObject(user);
});
```

### Middleware Composition

Compose multiple middleware into a single unit:

```cpp
// Middleware composition function
auto compose = [](std::initializer_list<boson::MiddlewareFunction> middlewares) {
    return [middlewares](const boson::Request& req, boson::Response& res, boson::NextFunction& outerNext) {
        // Create a vector from the initializer list
        std::vector<boson::MiddlewareFunction> middlewareVector(middlewares);
        
        // Function to execute middlewares in sequence
        std::function<void(size_t)> executeMiddleware;
        
        executeMiddleware = [&](size_t index) {
            if (index >= middlewareVector.size()) {
                // All middleware executed, continue to the next outer middleware
                outerNext();
                return;
            }
            
            // Create a next function that moves to the next middleware
            boson::NextFunction innerNext = [&executeMiddleware, index]() {
                executeMiddleware(index + 1);
            };
            
            // Execute the current middleware
            middlewareVector[index](req, res, innerNext);
        };
        
        // Start executing the first middleware
        executeMiddleware(0);
    };
};

// Usage example
auto apiMiddleware = compose({
    corsMiddleware,
    requestLogger,
    authenticate
});

// Apply the composed middleware
app.use("/api", apiMiddleware);
```

## Built-in Middleware

Boson provides several built-in middleware functions for common tasks:

### Body Parser

```cpp
// Add body parser middleware for different content types
app.use(boson::bodyParser.json());                  // application/json
app.use(boson::bodyParser.urlencoded());            // application/x-www-form-urlencoded
app.use(boson::bodyParser.text());                  // text/plain
app.use(boson::bodyParser.raw());                   // raw binary data
```

### Cookie Parser

```cpp
// Parse cookies in incoming requests
app.use(boson::cookieParser());

// Access parsed cookies in route handlers
app.get("/profile", [](const boson::Request& req, boson::Response& res) {
    std::string sessionId = req.cookie("sessionId");
    if (sessionId.empty()) {
        res.redirect("/login");
        return;
    }
    res.send("User profile");
});
```

### Static Files

```cpp
// Serve static files with default options
app.use(boson::staticFiles("public"));

// With custom options
std::unordered_map<std::string, std::string> options = {
    {"maxAge", "86400000"},                 // Cache for one day
    {"index", "index.html"},                // Default index file
    {"dotfiles", "ignore"},                 // Ignore dotfiles
    {"etag", "true"},                       // Enable ETag headers
    {"lastModified", "true"},               // Enable Last-Modified headers
    {"fallthrough", "true"}                 // Continue if file not found
};
app.use(boson::staticFiles("public", options));
```

### Compression

```cpp
// Add compression middleware
app.use(boson::compression({
    {"level", "6"},                        // Compression level (1-9)
    {"threshold", "1024"},                 // Only compress responses larger than this
    {"filter", "text/*, application/json"} // Only compress these content types
}));
```

## Middleware Visualization

To better understand the middleware execution flow, consider this diagram of a typical request through a Boson application:

```
┌───────────────────────────────────────────────────────────┐
│                      HTTP REQUEST                         │
└─────────────────────────────┬─────────────────────────────┘
                              │
                              ▼
┌───────────────────────────────────────────────────────────┐
│                Global Middleware (app.use())              │
│                                                           │
│  ┌───────────────┐  ┌──────────────┐  ┌─────────────────┐ │
│  │  Body Parser  │→ │ Cookie Parser│→ │ Request Logger  │→│
│  └───────────────┘  └──────────────┘  └─────────────────┘ │
│                                                           │
└─────────────────────────────┬─────────────────────────────┘
                              │
                              ▼
┌───────────────────────────────────────────────────────────┐
│             Path-Specific Middleware ("/api")             │
│                                                           │
│  ┌───────────────┐  ┌───────────────┐  ┌───────────────┐  │
│  │     CORS      │→ │ Authentication│→ │ Rate Limiter  │→ │
│  └───────────────┘  └───────────────┘  └───────────────┘  │
│                                                           │
└─────────────────────────────┬─────────────────────────────┘
                              │
                              ▼
┌───────────────────────────────────────────────────────────┐
│                Route Handler (app.get())                  │
│                                                           │
│  ┌───────────────────────────────────────────────────┐    │
│  │                Business Logic                     │    │
│  └───────────────────────────────────────────────────┘    │
│                                                           │
└─────────────────────────────┬─────────────────────────────┘
                              │
                              ▼
┌───────────────────────────────────────────────────────────┐
│              Response Processing Middleware               │
│                                                           │
│  ┌───────────────┐  ┌──────────────┐  ┌────────────────┐  │
│  │  Compression  │← │ Error Handler│← │  Response Time │← │
│  └───────────────┘  └──────────────┘  └────────────────┘  │
│                                                           │
└─────────────────────────────┬─────────────────────────────┘
                              │
                              ▼
┌───────────────────────────────────────────────────────────┐
│                      HTTP RESPONSE                        │
└───────────────────────────────────────────────────────────┘
```

## Middleware Best Practices

### 1. Keep Middleware Focused

Each middleware should have a single responsibility:

```cpp
// Good - Single responsibility
auto validateApiKey = [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    std::string apiKey = req.header("X-API-Key");
    if (apiKey.empty()) {
        res.status(401).jsonObject({{"error", "API key required"}});
        return;
    }
    next();
};

// Avoid - Doing too many things in one middleware
auto badMiddleware = [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    // Parsing body, validating API key, checking user permissions, etc.
    // Too many responsibilities in one middleware
};
```

### 2. Order Matters

Add middleware in the right order. Generally:
1. Logging/monitoring middleware first
2. Request parsing middleware (body parsers, etc.)
3. Security middleware (CORS, authentication, etc.)
4. Application-specific middleware
5. Error handling middleware last

```cpp
// Good middleware order
app.use(requestLogger);            // Log all requests immediately
app.use(boson::bodyParser.json()); // Parse request body
app.use(corsMiddleware);           // Handle CORS
app.use("/api", authenticate);     // Authenticate API routes
app.use(errorHandler);             // Catch errors
```

### 3. Always Handle Next

Always call `next()` unless you intend to end the request-response cycle:

```cpp
// Good - Always calls next() or sends a response
app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    if (someCondition()) {
        next();
    } else {
        res.status(400).send("Bad request");
    }
});

// Bad - Might forget to call next()
app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    if (someCondition()) {
        next();
    }
    // Missing else branch - request will hang if condition is false
});
```

### 4. Error Handling

Use try-catch blocks to handle errors in middleware:

```cpp
// Good - Handles errors properly
app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    try {
        // Something that might throw
        next();
    } catch (const std::exception& e) {
        res.status(500).jsonObject({{"error", e.what()}});
    }
});
```

### 5. Make Middleware Reusable

Design middleware to be reusable with configurable options:

```cpp
// Good - Reusable middleware with options
auto rateLimiter = [](int maxRequests, std::chrono::seconds window) {
    std::unordered_map<std::string, std::pair<int, std::chrono::steady_clock::time_point>> clients;
    
    return [=, &clients](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
        // Implementation with the provided options
    };
};

// Usage with different options
app.use("/api", rateLimiter(100, std::chrono::seconds(60)));  // 100 req/min for API
app.use("/login", rateLimiter(10, std::chrono::seconds(60))); // 10 req/min for login
```

### 6. Test Middleware

Write tests for your middleware functions:

```cpp
// Example test for middleware (pseudocode)
void testAuthMiddleware() {
    // Create mock request and response
    boson::Request mockReq;
    boson::Response mockRes;
    bool nextCalled = false;
    
    // Mock next function
    auto mockNext = [&nextCalled]() { nextCalled = true; };
    
    // Test with no token
    authenticate(mockReq, mockRes, mockNext);
    assert(mockRes.statusCode() == 401);
    assert(!nextCalled);
    
    // Test with valid token
    mockReq.setHeader("Authorization", "Bearer valid-token");
    nextCalled = false;
    mockRes.reset();
    authenticate(mockReq, mockRes, mockNext);
    assert(nextCalled);
    
    // More test cases...
}
```

### 7. Document Your Middleware

Clearly document your middleware, especially if they're reusable:

```cpp
/**
 * Rate limiting middleware that restricts the number of requests from a single IP.
 * 
 * @param maxRequests Maximum number of requests allowed within the time window
 * @param window Time window in seconds
 * @return Middleware function that implements rate limiting
 * 
 * Usage:
 *   app.use("/api", rateLimiter(100, std::chrono::seconds(60))); // 100 req/min
 */
auto rateLimiter = [](int maxRequests, std::chrono::seconds window) {
    // Implementation
};
```

### 8. Use Middleware for Cross-cutting Concerns

Identify cross-cutting concerns and implement them as middleware:

- Authentication and authorization
- Logging and monitoring
- Performance tracking
- Input validation
- Response formatting
- Error handling
- Security features (CORS, CSRF protection, etc.)

### 9. Be Mindful of Performance

Middleware runs on every matching request, so keep it efficient:

```cpp
// Good - Efficient middleware
app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    // Simple, fast operations
    std::cout << req.method() << " " << req.path() << std::endl;
    next();
});

// Avoid - Unnecessary complexity in frequently used middleware
app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    // Expensive operations on every request
    std::string requestBody = req.body();
    // Complex parsing, regex operations, file I/O, etc.
    next();
});
```

### 10. Consider Conditional Execution

Skip unnecessary middleware processing:

```cpp
// Skip middleware processing for certain paths
app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    // Skip processing for static files
    if (req.path().starts_with("/public/")) {
        next();
        return;
    }
    
    // Normal processing for other routes
    // ...
    next();
});
```

## Conclusion

Middleware is a powerful pattern that allows you to modularize your request-processing logic and keep your route handlers focused on their specific responsibilities. By mastering middleware in Boson, you can create clean, well-structured applications with proper separation of concerns.

The next section will cover [Request and Response](./request-response) objects in detail.
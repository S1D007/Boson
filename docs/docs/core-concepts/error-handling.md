---
sidebar_position: 6
title: Error Handling
---

# Error Handling in Boson

Proper error handling is essential for creating robust web applications. Boson provides several tools and patterns to help you handle errors effectively and provide useful responses to clients.

## Types of Errors

When building web applications with Boson, you'll encounter different types of errors:

1. **Client Errors**: Issues with client requests (4xx status codes)
2. **Server Errors**: Issues with your application logic (5xx status codes)
3. **Validation Errors**: Issues with request data validation
4. **Runtime Exceptions**: Unexpected exceptions during request handling
5. **Network Errors**: Issues with connections or external services

## Basic Error Handling

The simplest way to handle errors is directly within route handlers:

```cpp
app.get("/users/:id", [](const boson::Request& req, boson::Response& res) {
    try {
        std::string id = req.param("id");
        
        // Validate ID
        if (!isValidId(id)) {
            return res.status(400).jsonObject({
                {"error", "Invalid ID format"},
                {"message", "User ID must be a positive integer"}
            });
        }
        
        // Try to fetch the user
        User user = getUserById(id);
        
        // Check if user exists
        if (user.id.empty()) {
            return res.status(404).jsonObject({
                {"error", "Not Found"},
                {"message", "User with ID " + id + " does not exist"}
            });
        }
        
        // Success response
        res.jsonObject(user.toJson());
    } 
    catch (const std::exception& e) {
        // Server error
        res.status(500).jsonObject({
            {"error", "Internal Server Error"},
            {"message", "An unexpected error occurred"},
            {"details", e.what()}
        });
    }
});
```

## Using HttpError Class

Boson provides an `HttpError` class for more standardized error handling:

```cpp
#include <boson/error_handler.hpp>

app.get("/users/:id", [](const boson::Request& req, boson::Response& res) {
    try {
        std::string id = req.param("id");
        
        // Validate ID
        if (!isValidId(id)) {
            throw boson::HttpError("Invalid ID format", 400);
        }
        
        // Try to fetch the user
        User user = getUserById(id);
        
        // Check if user exists
        if (user.id.empty()) {
            throw boson::HttpError("User not found", 404);
        }
        
        // Success response
        res.jsonObject(user.toJson());
    } 
    catch (const boson::HttpError& e) {
        // Handle HTTP errors with proper status
        res.status(e.statusCode()).jsonObject({
            {"error", e.what()},
            {"status", e.statusCode()}
        });
    }
    catch (const std::exception& e) {
        // Handle other exceptions
        res.status(500).jsonObject({
            {"error", "Internal Server Error"},
            {"message", e.what()}
        });
    }
});
```

## Global Error Handler Middleware

For consistent error handling across your entire application, use error handler middleware:

```cpp
// Create a global error handler
auto errorHandler = [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    try {
        // Pass to next middleware or route handler
        next();
    }
    catch (const boson::HttpError& e) {
        // Handle HTTP errors
        res.status(e.statusCode()).jsonObject({
            {"error", e.what()},
            {"status", e.statusCode()},
            {"path", req.path()}
        });
    }
    catch (const std::exception& e) {
        // Handle standard exceptions
        res.status(500).jsonObject({
            {"error", "Internal Server Error"},
            {"message", e.what()},
            {"path", req.path()}
        });
    }
    catch (...) {
        // Handle unknown errors
        res.status(500).jsonObject({
            {"error", "Internal Server Error"},
            {"message", "An unknown error occurred"},
            {"path", req.path()}
        });
    }
};

// Add the error handler as the last middleware
app.use(errorHandler);

// Now routes can throw exceptions safely
app.get("/users/:id", [](const boson::Request& req, boson::Response& res) {
    std::string id = req.param("id");
    
    // Validate ID
    if (!isValidId(id)) {
        throw boson::HttpError("Invalid ID format", 400);
    }
    
    // Try to fetch the user
    User user = getUserById(id);
    
    // Check if user exists
    if (user.id.empty()) {
        throw boson::HttpError("User not found", 404);
    }
    
    // Success response
    res.jsonObject(user.toJson());
});
```

## Custom Error Types

You can create custom error types for specific error scenarios:

```cpp
// Define custom HTTP error types
class ValidationError : public boson::HttpError {
public:
    ValidationError(const std::string& message, 
                    const std::map<std::string, std::string>& errors)
        : boson::HttpError(message, 400), validationErrors(errors) {}
    
    std::map<std::string, std::string> validationErrors;
};

class NotFoundError : public boson::HttpError {
public:
    NotFoundError(const std::string& resource, const std::string& id)
        : boson::HttpError(resource + " with ID " + id + " not found", 404),
          resourceType(resource), resourceId(id) {}
          
    std::string resourceType;
    std::string resourceId;
};

class AuthorizationError : public boson::HttpError {
public:
    AuthorizationError(const std::string& message = "Unauthorized")
        : boson::HttpError(message, 401) {}
};

// Enhanced error handler that handles custom error types
auto errorHandler = [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    try {
        next();
    }
    catch (const ValidationError& e) {
        res.status(e.statusCode()).jsonObject({
            {"error", e.what()},
            {"status", e.statusCode()},
            {"validation_errors", e.validationErrors}
        });
    }
    catch (const NotFoundError& e) {
        res.status(e.statusCode()).jsonObject({
            {"error", e.what()},
            {"status", e.statusCode()},
            {"resource", e.resourceType},
            {"id", e.resourceId}
        });
    }
    catch (const AuthorizationError& e) {
        res.status(e.statusCode()).jsonObject({
            {"error", e.what()},
            {"status", e.statusCode()}
        });
    }
    // ... handle other error types and general errors
};

// Use custom error types in your routes
app.post("/users", [](const boson::Request& req, boson::Response& res) {
    nlohmann::json body = req.json();
    
    // Validate request body
    std::map<std::string, std::string> errors;
    
    if (!body.contains("username") || body["username"].empty()) {
        errors["username"] = "Username is required";
    }
    
    if (!body.contains("email") || body["email"].empty()) {
        errors["email"] = "Email is required";
    } else if (!isValidEmail(body["email"])) {
        errors["email"] = "Email is invalid";
    }
    
    if (!errors.empty()) {
        throw ValidationError("Validation failed", errors);
    }
    
    // Process valid request...
});
```

## Asynchronous Error Handling

For asynchronous operations, ensure you catch errors properly:

```cpp
app.get("/data", [](const boson::Request& req, boson::Response& res) {
    // Start an asynchronous operation
    fetchDataAsync()
        .then([&res](const Data& data) {
            // Success case
            res.jsonObject(data.toJson());
        })
        .catch([&res](const std::exception& e) {
            // Error case
            res.status(500).jsonObject({
                {"error", "Failed to fetch data"},
                {"message", e.what()}
            });
        });
});
```

## Validation Patterns

For input validation, consider creating reusable validation functions:

```cpp
// Validation utility
template <typename T>
void validate(const T& value, 
              const std::string& fieldName,
              std::map<std::string, std::string>& errors,
              const std::function<bool(const T&)>& validator,
              const std::string& errorMessage) {
    if (!validator(value)) {
        errors[fieldName] = errorMessage;
    }
}

// Using the validation utility
app.post("/register", [](const boson::Request& req, boson::Response& res) {
    auto body = req.json();
    std::map<std::string, std::string> errors;
    
    // Validate required fields
    if (body.contains("username")) {
        std::string username = body["username"];
        validate(username, "username", errors,
                [](const std::string& u) { return u.size() >= 3 && u.size() <= 20; },
                "Username must be between 3 and 20 characters");
    } else {
        errors["username"] = "Username is required";
    }
    
    if (body.contains("email")) {
        std::string email = body["email"];
        validate(email, "email", errors,
                [](const std::string& e) { return isValidEmail(e); },
                "Email address is invalid");
    } else {
        errors["email"] = "Email is required";
    }
    
    if (body.contains("password")) {
        std::string password = body["password"];
        validate(password, "password", errors,
                [](const std::string& p) { return p.size() >= 8; },
                "Password must be at least 8 characters");
    } else {
        errors["password"] = "Password is required";
    }
    
    // If validation fails, throw appropriate error
    if (!errors.empty()) {
        throw ValidationError("Validation failed", errors);
    }
    
    // Process valid registration...
});
```

## Logging Errors

Always log errors for debugging and monitoring:

```cpp
// Create an error logging middleware
auto errorLogger = [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    try {
        next();
    }
    catch (const std::exception& e) {
        // Log the error with important context
        std::cerr << "[ERROR] " 
                  << req.method() << " " << req.path() 
                  << " - " << e.what() 
                  << " - Client IP: " << req.ip() 
                  << std::endl;
        
        // Re-throw to be caught by the error handler
        throw;
    }
};

// Add the logger before the error handler
app.use(errorLogger);
app.use(errorHandler);
```

## Best Practices

1. **Use Centralized Error Handling**: Implement a global error handler
2. **Be Specific**: Use specific error types for different scenarios
3. **Hide Sensitive Information**: Don't expose stack traces or internal errors to clients
4. **Include Context**: Include relevant information in error responses
5. **Log Errors**: Always log errors for debugging
6. **Use Status Codes Correctly**: Use appropriate HTTP status codes
7. **Validation First**: Validate input before processing
8. **Consistent Format**: Maintain a consistent error response format

### Example Error Response Format

Adopting a consistent format makes errors easier to handle on the client side:

```json
{
  "error": "Validation failed",
  "status": 400,
  "timestamp": "2025-04-15T10:30:00Z",
  "path": "/api/users",
  "validation_errors": {
    "username": "Username must be between 3 and 20 characters",
    "email": "Email address is invalid"
  },
  "request_id": "a1b2c3d4-e5f6-7890-abcd-1234567890ab"
}
```

Implement this with:

```cpp
// Utility for consistent error responses
nlohmann::json createErrorResponse(const std::string& errorMessage, 
                                   int statusCode, 
                                   const boson::Request& req,
                                   const nlohmann::json& additionalData = {}) {
    nlohmann::json response = {
        {"error", errorMessage},
        {"status", statusCode},
        {"timestamp", getCurrentTimestamp()},
        {"path", req.path()},
        {"request_id", req.header("X-Request-ID", generateRequestId())}
    };
    
    // Add any additional error data
    for (auto& [key, value] : additionalData.items()) {
        response[key] = value;
    }
    
    return response;
}
```
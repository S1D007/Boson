---
sidebar_position: 2
title: Routing
---

# Routing in Boson

Routing is how you define the endpoints of your web application and specify how they respond to client requests. Boson makes routing simple and powerful, with support for path parameters, query parameters, and various HTTP methods.

## Basic Routes

Define routes using the HTTP method functions on your server instance:

```cpp
// Create a server instance
boson::Server app;

// Define a simple route
app.get("/hello", [](const boson::Request& req, boson::Response& res) {
    res.send("Hello, World!");
});

// Routes for different HTTP methods
app.post("/users", [](const boson::Request& req, boson::Response& res) {
    // Create a user...
    res.status(201).send("User created");
});

app.put("/users/:id", [](const boson::Request& req, boson::Response& res) {
    // Update a user...
    res.send("User updated");
});

app.del("/users/:id", [](const boson::Request& req, boson::Response& res) {
    // Delete a user...
    res.send("User deleted");
});

app.patch("/users/:id", [](const boson::Request& req, boson::Response& res) {
    // Partially update a user...
    res.send("User patched");
});

app.head("/status", [](const boson::Request& req, boson::Response& res) {
    // Headers only, no body
    res.status(200).send();
});

app.options("/api", [](const boson::Request& req, boson::Response& res) {
    // Respond to OPTIONS requests (commonly used for CORS)
    res.header("Allow", "GET, POST, PUT, DELETE")
       .status(200)
       .send();
});
```

## Route Parameters

You can define dynamic segments in your routes with parameters:

```cpp
// Route with a parameter (note the : prefix)
app.get("/users/:id", [](const boson::Request& req, boson::Response& res) {
    // Extract the parameter value
    std::string id = req.param("id");
    
    res.send("User ID: " + id);
});

// Multiple parameters
app.get("/posts/:year/:month/:slug", [](const boson::Request& req, boson::Response& res) {
    std::string year = req.param("year");
    std::string month = req.param("month");
    std::string slug = req.param("slug");
    
    res.send("Post from " + month + "/" + year + ": " + slug);
});
```

## Query Parameters

Handle query string parameters in your routes:

```cpp
// Route that handles query parameters (e.g., /search?q=boson&page=1)
app.get("/search", [](const boson::Request& req, boson::Response& res) {
    // Get a single query parameter
    std::string query = req.query("q");
    
    // Get a query parameter with a default value
    std::string page = req.query("page", "1");
    
    // Get all query parameters as a map
    auto allParams = req.queryParams();
    
    res.send("Searching for: " + query + " on page " + page);
});
```

## Route Groups

Group related routes together with routers:

```cpp
// Create a router for API routes
boson::Router apiRouter;

// Add routes to the router
apiRouter.get("/users", [](const boson::Request& req, boson::Response& res) {
    res.send("List of users");
});

apiRouter.post("/users", [](const boson::Request& req, boson::Response& res) {
    res.send("Create user");
});

// Mount the router at a base path
app.use("/api/v1", apiRouter);
// These routes are now accessible at /api/v1/users
```

## Route Handlers

A route handler is a function that is called when a request matches a route. There are several ways to define route handlers:

### Lambda Functions

```cpp
app.get("/hello", [](const boson::Request& req, boson::Response& res) {
    res.send("Hello, World!");
});
```

### Regular Functions

```cpp
void handleHello(const boson::Request& req, boson::Response& res) {
    res.send("Hello, World!");
}

// Use the function as a handler
app.get("/hello", handleHello);
```

### Class Methods

```cpp
class UserController {
public:
    void getUsers(const boson::Request& req, boson::Response& res) {
        res.send("List of users");
    }
};

UserController userController;
app.get("/users", std::bind(&UserController::getUsers, userController, 
                           std::placeholders::_1, std::placeholders::_2));

// Or with a static method
app.get("/users", &UserController::getUsers);
```

## Route Matching

Boson matches routes in the order they are defined. When a request comes in, Boson tries to match it against each route until it finds a matching one. If no route matches, it responds with a 404 error.

```cpp
// This will match /users/profile
app.get("/users/profile", [](const boson::Request& req, boson::Response& res) {
    res.send("User profile");
});

// This will match /users/:id, but only if the previous route didn't match
app.get("/users/:id", [](const boson::Request& req, boson::Response& res) {
    res.send("User details");
});

// Catch-all route
app.get("*", [](const boson::Request& req, boson::Response& res) {
    res.status(404).send("Not Found");
});
```

## Route-Specific Middleware

You can apply middleware to specific routes:

```cpp
// Authentication middleware
auto authenticate = [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    std::string token = req.header("Authorization");
    if (token.empty()) {
        res.status(401).send("Unauthorized");
        return;
    }
    // Authentication logic...
    next();
};

// Apply middleware to a specific route
app.get("/admin", authenticate, [](const boson::Request& req, boson::Response& res) {
    res.send("Admin dashboard");
});

// Apply middleware to a router
boson::Router adminRouter;
adminRouter.use(authenticate);
adminRouter.get("/dashboard", [](const boson::Request& req, boson::Response& res) {
    res.send("Admin dashboard");
});

app.use("/admin", adminRouter);
```

## Advanced Routing Patterns

### Version-Based Routing

Organize your API by versions:

```cpp
// Create routers for different API versions
boson::Router v1Router;
boson::Router v2Router;

v1Router.get("/users", [](const boson::Request& req, boson::Response& res) {
    res.jsonObject({{"version", "v1"}, {"users", "[]"}});
});

v2Router.get("/users", [](const boson::Request& req, boson::Response& res) {
    res.jsonObject({{"version", "v2"}, {"users", "[]"}, {"meta", {{"count", 0}}}});
});

// Mount the version routers
app.use("/api/v1", v1Router);
app.use("/api/v2", v2Router);
```

### Wildcard Routes

Handle a group of routes with wildcard patterns:

```cpp
// Match any path under /files/
app.get("/files/*", [](const boson::Request& req, boson::Response& res) {
    std::string filePath = req.wildcard();
    res.send("Requested file: " + filePath);
});
```

### Regular Expression Routes

Some complex routing patterns can be achieved with regular expressions:

```cpp
// Match only numeric IDs
app.get("/users/([0-9]+)", [](const boson::Request& req, boson::Response& res) {
    std::string id = req.regexMatch(1);  // Get the first capture group
    res.send("User with numeric ID: " + id);
});
```

## Best Practices

1. **Organize Routes Logically**: Group related routes under the same router
2. **Use Descriptive Route Names**: Make routes self-descriptive
3. **Follow REST Conventions**: Use appropriate HTTP methods
4. **Parameter Validation**: Always validate route parameters
5. **Keep Route Handlers Small**: Extract business logic to separate functions
6. **Order Routes Properly**: Put more specific routes before general ones
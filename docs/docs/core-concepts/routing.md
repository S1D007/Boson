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

### Parameter Validation

It's important to validate parameters to ensure they match expected formats:

```cpp
app.get("/users/:id", [](const boson::Request& req, boson::Response& res) {
    std::string id = req.param("id");
    
    // Validate that id is numeric
    if (!std::all_of(id.begin(), id.end(), ::isdigit)) {
        res.status(400).jsonObject({{"error", "Invalid user ID format"}});
        return;
    }
    
    // Convert to integer safely
    try {
        int userId = std::stoi(id);
        res.send("Valid user ID: " + std::to_string(userId));
    } catch (const std::exception& e) {
        res.status(400).jsonObject({{"error", "Invalid user ID"}});
    }
});
```

### Optional Parameters

You can make route segments optional by using parameter constraints:

```cpp
// Optional :page parameter (matches both /users and /users/2)
app.get("/users(/:page)?", [](const boson::Request& req, boson::Response& res) {
    std::string page = req.param("page", "1");  // Default to "1" if not provided
    res.send("Users page: " + page);
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

### Array Query Parameters

Handling array-like query parameters (e.g., `?tags[]=cpp&tags[]=web`):

```cpp
app.get("/filter", [](const boson::Request& req, boson::Response& res) {
    // Get array query parameters
    auto tags = req.queryArray("tags");
    
    std::string response = "Filtering by tags: ";
    for (size_t i = 0; i < tags.size(); ++i) {
        response += tags[i];
        if (i < tags.size() - 1) {
            response += ", ";
        }
    }
    
    res.send(response);
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

### Nested Routers

You can nest routers for more complex route organization:

```cpp
// Create routers for different resources
boson::Router usersRouter;
boson::Router postsRouter;
boson::Router apiRouter;

// Define user routes
usersRouter.get("/", [](const boson::Request& req, boson::Response& res) {
    res.send("List users");
});
usersRouter.get("/:id", [](const boson::Request& req, boson::Response& res) {
    res.send("Get user " + req.param("id"));
});

// Define post routes
postsRouter.get("/", [](const boson::Request& req, boson::Response& res) {
    res.send("List posts");
});
postsRouter.get("/:id", [](const boson::Request& req, boson::Response& res) {
    res.send("Get post " + req.param("id"));
});

// Mount resource routers to the API router
apiRouter.use("/users", usersRouter);
apiRouter.use("/posts", postsRouter);

// Mount the API router to the main app
app.use("/api", apiRouter);

// This creates routes like:
// - /api/users
// - /api/users/:id
// - /api/posts
// - /api/posts/:id
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

### Multiple Handlers

You can chain multiple handlers for a single route:

```cpp
// First validate the request
auto validateUser = [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    if (!req.json().contains("name")) {
        res.status(400).jsonObject({{"error", "Name is required"}});
        return;
    }
    next();
};

// Then process it
auto createUser = [](const boson::Request& req, boson::Response& res) {
    auto data = req.json();
    res.status(201).jsonObject({{"message", "User created"}, {"name", data["name"]}});
};

// Register both handlers for the route
app.post("/users", validateUser, createUser);
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

### How Route Matching Works

Boson uses a priority-based algorithm for route matching:

1. **Static routes** (like `/users`) are checked first
2. **Parameterized routes** (like `/users/:id`) are checked next
3. **Wildcard routes** (like `/users/*`) are checked last

Within each category, routes are matched in the order they were defined.

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

### Chainable Route Definitions

Define multiple HTTP methods for the same path:

```cpp
app.route("/users/:id")
   .get([](const boson::Request& req, boson::Response& res) {
       res.send("Get user " + req.param("id"));
   })
   .put([](const boson::Request& req, boson::Response& res) {
       res.send("Update user " + req.param("id"));
   })
   .del([](const boson::Request& req, boson::Response& res) {
       res.send("Delete user " + req.param("id"));
   });
```

### Resource-Based Routing

Quickly define RESTful resource routes:

```cpp
// Create a controller for the resource
class UserController {
public:
    // List all users
    void index(const boson::Request& req, boson::Response& res) {
        res.send("List all users");
    }
    
    // Show a single user
    void show(const boson::Request& req, boson::Response& res) {
        res.send("Show user " + req.param("id"));
    }
    
    // Create a new user
    void create(const boson::Request& req, boson::Response& res) {
        res.status(201).send("Create a user");
    }
    
    // Update a user
    void update(const boson::Request& req, boson::Response& res) {
        res.send("Update user " + req.param("id"));
    }
    
    // Delete a user
    void destroy(const boson::Request& req, boson::Response& res) {
        res.send("Delete user " + req.param("id"));
    }
};

// Register resource routes (creates multiple routes automatically)
UserController userController;
app.resource("/users", userController);

// This creates:
// GET /users - userController.index
// GET /users/:id - userController.show
// POST /users - userController.create
// PUT /users/:id - userController.update
// DELETE /users/:id - userController.destroy
```

## Route Visualization

Understanding your application's route structure is important. Here's a visual representation of a typical route hierarchy:

```
Server (app)
│
├── GET    /                         - Homepage
├── GET    /about                    - About page
│
├── /api   (apiRouter)
│   │
│   ├── /v1  (v1Router)
│   │   ├── GET    /users           - List users (v1)
│   │   ├── POST   /users           - Create user (v1)
│   │   ├── GET    /users/:id       - Get user (v1)
│   │   └── DELETE /users/:id       - Delete user (v1)
│   │
│   └── /v2  (v2Router)
│       ├── GET    /users           - List users (v2)
│       ├── GET    /users/:id       - Get user (v2)
│       └── ...
│
├── /admin  (adminRouter) [protected by auth middleware]
│   ├── GET    /                    - Admin dashboard
│   └── GET    /settings            - Admin settings
│
└── *                               - 404 catch-all route
```

## Best Practices

### 1. Organize Routes Logically

Group related routes together using routers:

```cpp
// User-related routes
boson::Router userRouter;
userRouter.get("/", listUsersHandler);
userRouter.post("/", createUserHandler);
userRouter.get("/:id", getUserHandler);
// ... more user routes

// Post-related routes
boson::Router postRouter;
postRouter.get("/", listPostsHandler);
postRouter.post("/", createPostHandler);
// ... more post routes

// Mount routers
app.use("/users", userRouter);
app.use("/posts", postRouter);
```

### 2. Use Descriptive Route Names

Make routes self-descriptive following REST conventions:

```cpp
// Good - descriptive of the resource and action
app.get("/users", listUsersHandler);
app.post("/users", createUserHandler);
app.get("/users/:id", getUserHandler);
app.put("/users/:id", updateUserHandler);
app.del("/users/:id", deleteUserHandler);

// Avoid - unclear what these routes do
app.get("/getUsers", getUsersHandler);
app.post("/createUser", createUserHandler);
app.get("/user-details", userDetailsHandler);
```

### 3. Follow REST Conventions

Use appropriate HTTP methods for CRUD operations:

- `GET` - Read a resource or collection
- `POST` - Create a new resource
- `PUT` - Update a resource (full replacement)
- `PATCH` - Update a resource (partial update)
- `DELETE` - Remove a resource

### 4. Parameter Validation

Always validate route parameters:

```cpp
app.get("/users/:id", [](const boson::Request& req, boson::Response& res) {
    std::string id = req.param("id");
    
    // Validate
    if (id.empty() || !std::all_of(id.begin(), id.end(), ::isdigit)) {
        res.status(400).jsonObject({{"error", "Invalid user ID"}});
        return;
    }
    
    // Process valid ID...
});
```

### 5. Keep Route Handlers Small

Extract business logic to separate functions:

```cpp
// Good - route handler delegates to service functions
app.get("/users/:id", [](const boson::Request& req, boson::Response& res) {
    std::string id = req.param("id");
    
    try {
        auto user = UserService::findById(id);
        res.jsonObject(user.toJson());
    } catch (const UserNotFoundException& e) {
        res.status(404).jsonObject({{"error", "User not found"}});
    } catch (const std::exception& e) {
        res.status(500).jsonObject({{"error", "Internal server error"}});
    }
});

// Avoid - complex business logic in route handlers
app.get("/users/:id", [](const boson::Request& req, boson::Response& res) {
    std::string id = req.param("id");
    
    // Database connection, validation, error handling, etc...
    // [many lines of complex logic]
});
```

### 6. Order Routes Properly

Put more specific routes before general ones:

```cpp
// Specific route first
app.get("/users/new", newUserFormHandler);

// Parameterized route after
app.get("/users/:id", getUserHandler);

// General route last
app.get("/users", listUsersHandler);
```

### 7. Use Middleware for Cross-Cutting Concerns

Apply middleware for authentication, logging, error handling, etc.:

```cpp
// Authentication middleware
auto requireAuth = [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    // Authentication logic
    if (!isAuthenticated(req)) {
        res.status(401).jsonObject({{"error", "Unauthorized"}});
        return;
    }
    next();
};

// Apply to specific routes or routers
adminRouter.use(requireAuth);

// Or to specific routes
app.get("/profile", requireAuth, profileHandler);
```

### 8. Use Named Route Parameters

Make route parameters descriptive:

```cpp
// Good - descriptive parameter names
app.get("/posts/:year/:month/:slug", postHandler);

// Avoid - unclear parameter names
app.get("/posts/:p1/:p2/:p3", postHandler);
```

### 9. Consider Rate Limiting for Public APIs

Implement rate limiting to protect your API:

```cpp
// Simple rate limiting middleware
auto rateLimit = [](int maxRequests, std::chrono::seconds window) {
    std::unordered_map<std::string, std::pair<int, std::chrono::steady_clock::time_point>> clients;
    
    return [=, &clients](const boson::Request& req, boson::Response& res, boson::NextFunction& next) mutable {
        std::string clientIp = req.ip();
        auto now = std::chrono::steady_clock::now();
        
        // Clean up old entries
        for (auto it = clients.begin(); it != clients.end();) {
            if (now - it->second.second > window) {
                it = clients.erase(it);
            } else {
                ++it;
            }
        }
        
        // Check rate limit
        auto& client = clients[clientIp];
        if (client.first >= maxRequests && now - client.second < window) {
            res.status(429).jsonObject({{"error", "Too many requests"}});
            return;
        }
        
        // Update client count
        if (now - client.second < window) {
            client.first++;
        } else {
            client = {1, now};
        }
        
        next();
    };
};

// Apply rate limiting to API routes
apiRouter.use(rateLimit(100, std::chrono::seconds(60))); // 100 requests per minute
```

### 10. Document Your Routes

Comment your routes for better maintainability:

```cpp
// User management endpoints
// --------------------------

// List all users with pagination
// GET /users?page=1&limit=20
app.get("/users", [](const boson::Request& req, boson::Response& res) {
    std::string page = req.query("page", "1");
    std::string limit = req.query("limit", "20");
    // Implementation...
});

// Get user by ID
// GET /users/:id
app.get("/users/:id", [](const boson::Request& req, boson::Response& res) {
    std::string id = req.param("id");
    // Implementation...
});
```

## Route Testing Strategies

When implementing routes, consider these testing approaches:

1. **Unit Testing**: Test individual route handlers with mocked request/response objects
2. **Integration Testing**: Test route chains including middleware
3. **End-to-End Testing**: Test the full HTTP request/response cycle

## Common Routing Patterns

### Authentication Routes

```cpp
// Authentication routes
app.post("/login", loginHandler);
app.post("/register", registerHandler);
app.post("/logout", logoutHandler);
app.get("/forgot-password", forgotPasswordFormHandler);
app.post("/forgot-password", forgotPasswordHandler);
app.get("/reset-password/:token", resetPasswordFormHandler);
app.post("/reset-password/:token", resetPasswordHandler);
```

### API CRUD Routes

```cpp
// CRUD API for resources
app.get("/api/resources", listResourcesHandler);
app.post("/api/resources", createResourceHandler);
app.get("/api/resources/:id", getResourceHandler);
app.put("/api/resources/:id", updateResourceHandler);
app.patch("/api/resources/:id", partialUpdateResourceHandler);
app.del("/api/resources/:id", deleteResourceHandler);
```

### Nested Resources

```cpp
// Nested resources (e.g., posts belonging to users)
app.get("/users/:userId/posts", getUserPostsHandler);
app.post("/users/:userId/posts", createUserPostHandler);
app.get("/users/:userId/posts/:postId", getUserPostHandler);
```

## Conclusion

Effective routing is at the core of any web application. Boson provides a flexible and intuitive routing system that can handle simple to complex routing scenarios. By following the best practices outlined in this guide, you'll build maintainable and scalable route structures for your applications.

Next, learn about [Middleware](./middleware) to enhance your routes with additional functionality.
---
sidebar_position: 5
title: Controllers
---

# Controllers

Controllers provide a way to organize related routes and their handling logic. In Boson, controllers help you structure your application by grouping functionality around a specific resource or concept.

## What are Controllers?

Controllers are classes that group related route handlers together. They help organize your code by separating concerns and providing a cohesive API for a specific resource or domain concept.

## Basic Controller

A basic controller in Boson extends the `boson::Controller` class and implements the required methods:

```cpp
#include <boson/boson.hpp>
#include <boson/controller.hpp>

class UserController : public boson::Controller {
public:
    // Define the base path for all routes in this controller
    std::string basePath() const override {
        return "/users";
    }
    
    // List all users
    void getUsers(const boson::Request& req, boson::Response& res) {
        // Fetch users from database...
        nlohmann::json users = nlohmann::json::array({
            {{"id", 1}, {"name", "John Doe"}},
            {{"id", 2}, {"name", "Jane Smith"}}
        });
        
        res.jsonObject({{"users", users}});
    }
    
    // Get a single user by ID
    void getUserById(const boson::Request& req, boson::Response& res) {
        std::string id = req.param("id");
        
        // Fetch user from database...
        nlohmann::json user = {
            {"id", id},
            {"name", "John Doe"},
            {"email", "john@example.com"}
        };
        
        res.jsonObject(user);
    }
    
    // Create a new user
    void createUser(const boson::Request& req, boson::Response& res) {
        nlohmann::json body = req.json();
        
        // Create user in database...
        nlohmann::json user = {
            {"id", 3},
            {"name", body["name"]},
            {"email", body["email"]}
        };
        
        res.status(201).jsonObject({
            {"message", "User created successfully"},
            {"user", user}
        });
    }
    
    // Update a user
    void updateUser(const boson::Request& req, boson::Response& res) {
        std::string id = req.param("id");
        nlohmann::json body = req.json();
        
        // Update user in database...
        nlohmann::json user = {
            {"id", id},
            {"name", body["name"]},
            {"email", body["email"]}
        };
        
        res.jsonObject({
            {"message", "User updated successfully"},
            {"user", user}
        });
    }
    
    // Delete a user
    void deleteUser(const boson::Request& req, boson::Response& res) {
        std::string id = req.param("id");
        
        // Delete user from database...
        
        res.jsonObject({
            {"message", "User deleted successfully"},
            {"id", id}
        });
    }
};
```

## Registering Controller Routes

There are two main ways to register controller routes with your application:

### 1. Using RouteBinder

The `boson::createRouter` function uses the RouteBinder to map controller methods to routes:

```cpp
int main() {
    boson::initialize();
    boson::Server app;
    
    // Create controller instance
    auto userController = std::make_shared<UserController>();
    
    // Create router for the controller
    auto userRouter = boson::createRouter(userController);
    
    // Define routes using method chaining
    userRouter.get("/", &UserController::getUsers)
             .get("/:id", &UserController::getUserById)
             .post("/", &UserController::createUser)
             .put("/:id", &UserController::updateUser)
             .del("/:id", &UserController::deleteUser);
    
    // Mount the router on the application
    userRouter.mountOn(&app);
    
    app.configure(3000, "127.0.0.1");
    return app.listen();
}
```

### 2. Manual Registration

You can also manually register controller methods:

```cpp
int main() {
    boson::initialize();
    boson::Server app;
    
    // Create controller instance
    auto userController = std::make_shared<UserController>();
    
    // Define the base path
    std::string basePath = userController->basePath();
    
    // Register routes manually
    app.get(basePath, 
        std::bind(&UserController::getUsers, userController, 
                 std::placeholders::_1, std::placeholders::_2));
                 
    app.get(basePath + "/:id", 
        std::bind(&UserController::getUserById, userController, 
                 std::placeholders::_1, std::placeholders::_2));
                 
    app.post(basePath, 
        std::bind(&UserController::createUser, userController, 
                 std::placeholders::_1, std::placeholders::_2));
                 
    app.put(basePath + "/:id", 
        std::bind(&UserController::updateUser, userController, 
                 std::placeholders::_1, std::placeholders::_2));
                 
    app.del(basePath + "/:id", 
        std::bind(&UserController::deleteUser, userController, 
                 std::placeholders::_1, std::placeholders::_2));
    
    app.configure(3000, "127.0.0.1");
    return app.listen();
}
```

## Advanced Controller Features

### Controller Middleware

You can add middleware that applies to all routes in a controller:

```cpp
class UserController : public boson::Controller {
public:
    UserController() {
        // Add middleware in constructor
        addMiddleware([this](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
            std::cout << "Controller middleware: " << req.path() << std::endl;
            next();
        });
    }
    
    std::string basePath() const override {
        return "/users";
    }
    
    // Override to provide middleware for controller
    std::vector<boson::MiddlewareFunction> middleware() const override {
        return {
            // Authentication middleware for all controller routes
            [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
                std::string token = req.header("Authorization");
                if (token.empty()) {
                    res.status(401).jsonObject({{"error", "Unauthorized"}});
                    return;
                }
                next();
            },
            
            // Logging middleware
            [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
                std::cout << "Request to UserController: " << req.path() << std::endl;
                next();
            }
        };
    }
    
    // Controller methods...
};
```

### Route-Specific Middleware

You can also add middleware for specific routes:

```cpp
int main() {
    // ...
    
    auto adminOnly = [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
        // Check if user is admin
        if (!isAdmin(req)) {
            res.status(403).jsonObject({{"error", "Admin access required"}});
            return;
        }
        next();
    };
    
    auto userController = std::make_shared<UserController>();
    auto userRouter = boson::createRouter(userController);
    
    // Apply middleware to specific routes
    userRouter.get("/", &UserController::getUsers)
             .get("/:id", &UserController::getUserById)
             .post("/", adminOnly, &UserController::createUser)  // Only admins can create users
             .put("/:id", adminOnly, &UserController::updateUser)  // Only admins can update users
             .del("/:id", adminOnly, &UserController::deleteUser);  // Only admins can delete users
    
    userRouter.mountOn(&app);
    
    // ...
}
```

## Controller Organization Patterns

### Resource Controllers

Organize controllers around resources (nouns) with standard CRUD operations:

```cpp
class ProductController : public boson::Controller {
public:
    std::string basePath() const override {
        return "/products";
    }
    
    void index(const boson::Request& req, boson::Response& res) {
        // GET /products - List all products
    }
    
    void show(const boson::Request& req, boson::Response& res) {
        // GET /products/:id - Show a specific product
    }
    
    void create(const boson::Request& req, boson::Response& res) {
        // POST /products - Create a new product
    }
    
    void update(const boson::Request& req, boson::Response& res) {
        // PUT /products/:id - Update a product
    }
    
    void remove(const boson::Request& req, boson::Response& res) {
        // DELETE /products/:id - Delete a product
    }
};
```

### Feature Controllers

Organize controllers around specific features or use cases:

```cpp
class AuthController : public boson::Controller {
public:
    std::string basePath() const override {
        return "/auth";
    }
    
    void login(const boson::Request& req, boson::Response& res) {
        // POST /auth/login
    }
    
    void signup(const boson::Request& req, boson::Response& res) {
        // POST /auth/signup
    }
    
    void logout(const boson::Request& req, boson::Response& res) {
        // POST /auth/logout
    }
    
    void resetPassword(const boson::Request& req, boson::Response& res) {
        // POST /auth/reset-password
    }
};
```

## Using Multiple Controllers

For larger applications, you'll typically have multiple controllers:

```cpp
int main() {
    boson::initialize();
    boson::Server app;
    
    // Create and register multiple controllers
    auto userController = std::make_shared<UserController>();
    auto productController = std::make_shared<ProductController>();
    auto authController = std::make_shared<AuthController>();
    
    // Set up user routes
    auto userRouter = boson::createRouter(userController);
    userRouter.get("/", &UserController::getUsers)
             .get("/:id", &UserController::getUserById)
             .post("/", &UserController::createUser)
             .put("/:id", &UserController::updateUser)
             .del("/:id", &UserController::deleteUser);
    
    // Set up product routes
    auto productRouter = boson::createRouter(productController);
    productRouter.get("/", &ProductController::index)
                .get("/:id", &ProductController::show)
                .post("/", &ProductController::create)
                .put("/:id", &ProductController::update)
                .del("/:id", &ProductController::remove);
    
    // Set up auth routes
    auto authRouter = boson::createRouter(authController);
    authRouter.post("/login", &AuthController::login)
             .post("/signup", &AuthController::signup)
             .post("/logout", &AuthController::logout)
             .post("/reset-password", &AuthController::resetPassword);
    
    // Mount all routers
    userRouter.mountOn(&app);
    productRouter.mountOn(&app);
    authRouter.mountOn(&app);
    
    app.configure(3000, "127.0.0.1");
    return app.listen();
}
```

## Controller Best Practices

1. **Single Responsibility**: Each controller should handle a single resource or domain concept
2. **Consistent Naming**: Use consistent naming for controller methods (e.g., index, show, create, update, delete)
3. **Keep Controllers Thin**: Move business logic to service classes
4. **Use Dependency Injection**: Pass dependencies to controllers via constructors
5. **Validation**: Validate request data before processing
6. **Error Handling**: Use try-catch blocks for robust error handling

### Example with Dependency Injection and Services

```cpp
// Service class for business logic
class UserService {
public:
    std::vector<User> getUsers() {
        // Database access, business logic, etc.
    }
    
    User getUserById(int id) {
        // Database access, business logic, etc.
    }
    
    // Other methods...
};

// Controller using the service
class UserController : public boson::Controller {
private:
    UserService& userService;
    
public:
    // Inject dependencies via constructor
    UserController(UserService& service) : userService(service) {}
    
    std::string basePath() const override {
        return "/users";
    }
    
    void getUsers(const boson::Request& req, boson::Response& res) {
        try {
            auto users = userService.getUsers();
            
            nlohmann::json jsonUsers = nlohmann::json::array();
            for (const auto& user : users) {
                jsonUsers.push_back(user.toJson());
            }
            
            res.jsonObject({{"users", jsonUsers}});
        } catch (const std::exception& e) {
            res.status(500).jsonObject({{"error", e.what()}});
        }
    }
    
    // Other methods...
};

int main() {
    // Create services
    UserService userService;
    
    // Create controllers with injected services
    auto userController = std::make_shared<UserController>(userService);
    
    // Set up routes...
}
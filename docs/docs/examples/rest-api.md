---
sidebar_position: 1
title: Building a REST API
---

# Building a Complete REST API

This example shows how to build a complete REST API using the Boson framework with proper organization, error handling, and validation.

## Project Structure

For this example, we'll use the following structure:

```
rest-api/
├── CMakeLists.txt
├── config/
│   └── app.json
├── src/
│   ├── main.cpp
│   ├── controllers/
│   │   ├── user_controller.hpp
│   │   └── product_controller.hpp
│   ├── models/
│   │   ├── user.hpp
│   │   └── product.hpp
│   ├── services/
│   │   ├── user_service.hpp
│   │   └── product_service.hpp
│   ├── middleware/
│   │   ├── auth_middleware.hpp
│   │   └── validation_middleware.hpp
│   └── utils/
│       ├── error_handler.hpp
│       └── config_loader.hpp
```

## Step 1: Creating Models

Let's start by defining our data models:

```cpp
// src/models/user.hpp
#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class User {
public:
    int id;
    std::string name;
    std::string email;
    std::string passwordHash;
    std::vector<std::string> roles;
    std::string createdAt;
    std::string updatedAt;
    
    // Convert to JSON object
    nlohmann::json toJson(bool includePrivate = false) const {
        nlohmann::json json = {
            {"id", id},
            {"name", name},
            {"email", email},
            {"roles", roles},
            {"createdAt", createdAt},
            {"updatedAt", updatedAt}
        };
        
        if (includePrivate) {
            json["passwordHash"] = passwordHash;
        }
        
        return json;
    }
    
    // Create from JSON
    static User fromJson(const nlohmann::json& json) {
        User user;
        user.id = json.contains("id") ? json["id"].get<int>() : 0;
        user.name = json.contains("name") ? json["name"].get<std::string>() : "";
        user.email = json.contains("email") ? json["email"].get<std::string>() : "";
        user.passwordHash = json.contains("passwordHash") ? json["passwordHash"].get<std::string>() : "";
        
        if (json.contains("roles") && json["roles"].is_array()) {
            user.roles = json["roles"].get<std::vector<std::string>>();
        }
        
        user.createdAt = json.contains("createdAt") ? json["createdAt"].get<std::string>() : "";
        user.updatedAt = json.contains("updatedAt") ? json["updatedAt"].get<std::string>() : "";
        
        return user;
    }
    
    // Validate user data for creation
    static std::map<std::string, std::string> validate(const nlohmann::json& json) {
        std::map<std::string, std::string> errors;
        
        // Check required fields
        if (!json.contains("name") || json["name"].empty()) {
            errors["name"] = "Name is required";
        } else if (json["name"].get<std::string>().length() < 2) {
            errors["name"] = "Name must be at least 2 characters long";
        }
        
        if (!json.contains("email") || json["email"].empty()) {
            errors["email"] = "Email is required";
        } else if (!isValidEmail(json["email"].get<std::string>())) {
            errors["email"] = "Email format is invalid";
        }
        
        if (!json.contains("password") || json["password"].empty()) {
            errors["password"] = "Password is required";
        } else if (json["password"].get<std::string>().length() < 8) {
            errors["password"] = "Password must be at least 8 characters long";
        }
        
        return errors;
    }
    
private:
    // Simple email validation
    static bool isValidEmail(const std::string& email) {
        // Basic validation - in a real app, use a proper regex
        return email.find('@') != std::string::npos && 
               email.find('.') != std::string::npos;
    }
};

// src/models/product.hpp
#pragma once
#include <string>
#include <nlohmann/json.hpp>

class Product {
public:
    int id;
    std::string name;
    std::string description;
    double price;
    int stock;
    std::string category;
    std::string createdAt;
    std::string updatedAt;
    
    // Convert to JSON object
    nlohmann::json toJson() const {
        return {
            {"id", id},
            {"name", name},
            {"description", description},
            {"price", price},
            {"stock", stock},
            {"category", category},
            {"createdAt", createdAt},
            {"updatedAt", updatedAt}
        };
    }
    
    // Create from JSON
    static Product fromJson(const nlohmann::json& json) {
        Product product;
        product.id = json.contains("id") ? json["id"].get<int>() : 0;
        product.name = json.contains("name") ? json["name"].get<std::string>() : "";
        product.description = json.contains("description") ? json["description"].get<std::string>() : "";
        product.price = json.contains("price") ? json["price"].get<double>() : 0.0;
        product.stock = json.contains("stock") ? json["stock"].get<int>() : 0;
        product.category = json.contains("category") ? json["category"].get<std::string>() : "";
        product.createdAt = json.contains("createdAt") ? json["createdAt"].get<std::string>() : "";
        product.updatedAt = json.contains("updatedAt") ? json["updatedAt"].get<std::string>() : "";
        
        return product;
    }
    
    // Validate product data
    static std::map<std::string, std::string> validate(const nlohmann::json& json) {
        std::map<std::string, std::string> errors;
        
        if (!json.contains("name") || json["name"].empty()) {
            errors["name"] = "Name is required";
        }
        
        if (json.contains("price")) {
            if (!json["price"].is_number()) {
                errors["price"] = "Price must be a number";
            } else if (json["price"].get<double>() < 0) {
                errors["price"] = "Price cannot be negative";
            }
        } else {
            errors["price"] = "Price is required";
        }
        
        if (json.contains("stock")) {
            if (!json["stock"].is_number()) {
                errors["stock"] = "Stock must be a number";
            } else if (json["stock"].get<int>() < 0) {
                errors["stock"] = "Stock cannot be negative";
            }
        } else {
            errors["stock"] = "Stock is required";
        }
        
        return errors;
    }
};
```

## Step 2: Creating Services

Next, let's create services to handle business logic:

```cpp
// src/services/user_service.hpp
#pragma once
#include <vector>
#include <optional>
#include <algorithm>
#include <stdexcept>
#include "../models/user.hpp"

// In a real application, this would interact with a database
class UserService {
private:
    // In-memory storage for this example
    std::vector<User> users;
    int nextId = 1;
    
    // Find user by ID
    auto findUserById(int id) {
        return std::find_if(users.begin(), users.end(), 
                           [id](const User& user) { return user.id == id; });
    }
    
    // Find user by email
    auto findUserByEmail(const std::string& email) {
        return std::find_if(users.begin(), users.end(), 
                           [&email](const User& user) { return user.email == email; });
    }
    
public:
    UserService() {
        // Add some sample users
        User admin;
        admin.id = nextId++;
        admin.name = "Admin User";
        admin.email = "admin@example.com";
        admin.passwordHash = "hashed_password_here"; // In reality, use a proper hashing function
        admin.roles = {"admin", "user"};
        admin.createdAt = "2025-04-15T00:00:00Z";
        admin.updatedAt = "2025-04-15T00:00:00Z";
        users.push_back(admin);
        
        User regularUser;
        regularUser.id = nextId++;
        regularUser.name = "Regular User";
        regularUser.email = "user@example.com";
        regularUser.passwordHash = "hashed_password_here";
        regularUser.roles = {"user"};
        regularUser.createdAt = "2025-04-15T00:00:00Z";
        regularUser.updatedAt = "2025-04-15T00:00:00Z";
        users.push_back(regularUser);
    }
    
    // Get all users
    std::vector<User> getAllUsers() {
        return users;
    }
    
    // Get user by ID
    std::optional<User> getUserById(int id) {
        auto it = findUserById(id);
        if (it != users.end()) {
            return *it;
        }
        return std::nullopt;
    }
    
    // Create a new user
    User createUser(const User& user) {
        // Check if email already exists
        auto it = findUserByEmail(user.email);
        if (it != users.end()) {
            throw std::runtime_error("Email already in use");
        }
        
        // Create new user
        User newUser = user;
        newUser.id = nextId++;
        newUser.createdAt = getCurrentTimestamp();
        newUser.updatedAt = newUser.createdAt;
        
        // In a real application, hash the password
        
        users.push_back(newUser);
        return newUser;
    }
    
    // Update a user
    std::optional<User> updateUser(int id, const User& updatedUser) {
        auto it = findUserById(id);
        if (it == users.end()) {
            return std::nullopt;
        }
        
        // Check if email is being changed and if it's already in use
        if (it->email != updatedUser.email) {
            auto emailIt = findUserByEmail(updatedUser.email);
            if (emailIt != users.end() && emailIt->id != id) {
                throw std::runtime_error("Email already in use");
            }
        }
        
        // Update user fields
        it->name = updatedUser.name;
        it->email = updatedUser.email;
        if (!updatedUser.passwordHash.empty()) {
            it->passwordHash = updatedUser.passwordHash;
        }
        it->roles = updatedUser.roles;
        it->updatedAt = getCurrentTimestamp();
        
        return *it;
    }
    
    // Delete a user
    bool deleteUser(int id) {
        auto it = findUserById(id);
        if (it == users.end()) {
            return false;
        }
        
        users.erase(it);
        return true;
    }
    
    // Authenticate a user
    std::optional<User> authenticate(const std::string& email, const std::string& password) {
        auto it = findUserByEmail(email);
        if (it == users.end()) {
            return std::nullopt;
        }
        
        // In a real application, hash the password and compare with stored hash
        // This is just a simplified example
        if (it->passwordHash == password) {
            return *it;
        }
        
        return std::nullopt;
    }
    
private:
    // Get current timestamp in ISO format
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&now_c), "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    }
};

// src/services/product_service.hpp
#pragma once
#include <vector>
#include <optional>
#include <algorithm>
#include "../models/product.hpp"

// In a real application, this would interact with a database
class ProductService {
private:
    // In-memory storage for this example
    std::vector<Product> products;
    int nextId = 1;
    
    // Find product by ID
    auto findProductById(int id) {
        return std::find_if(products.begin(), products.end(), 
                           [id](const Product& product) { return product.id == id; });
    }
    
public:
    ProductService() {
        // Add some sample products
        Product product1;
        product1.id = nextId++;
        product1.name = "Product 1";
        product1.description = "Description for Product 1";
        product1.price = 19.99;
        product1.stock = 100;
        product1.category = "Electronics";
        product1.createdAt = "2025-04-15T00:00:00Z";
        product1.updatedAt = "2025-04-15T00:00:00Z";
        products.push_back(product1);
        
        Product product2;
        product2.id = nextId++;
        product2.name = "Product 2";
        product2.description = "Description for Product 2";
        product2.price = 29.99;
        product2.stock = 50;
        product2.category = "Home";
        product2.createdAt = "2025-04-15T00:00:00Z";
        product2.updatedAt = "2025-04-15T00:00:00Z";
        products.push_back(product2);
    }
    
    // Get all products
    std::vector<Product> getAllProducts() {
        return products;
    }
    
    // Get products by category
    std::vector<Product> getProductsByCategory(const std::string& category) {
        std::vector<Product> result;
        std::copy_if(products.begin(), products.end(), std::back_inserter(result),
                    [&category](const Product& product) { return product.category == category; });
        return result;
    }
    
    // Get product by ID
    std::optional<Product> getProductById(int id) {
        auto it = findProductById(id);
        if (it != products.end()) {
            return *it;
        }
        return std::nullopt;
    }
    
    // Create a new product
    Product createProduct(const Product& product) {
        Product newProduct = product;
        newProduct.id = nextId++;
        newProduct.createdAt = getCurrentTimestamp();
        newProduct.updatedAt = newProduct.createdAt;
        
        products.push_back(newProduct);
        return newProduct;
    }
    
    // Update a product
    std::optional<Product> updateProduct(int id, const Product& updatedProduct) {
        auto it = findProductById(id);
        if (it == products.end()) {
            return std::nullopt;
        }
        
        // Update product fields
        it->name = updatedProduct.name;
        it->description = updatedProduct.description;
        it->price = updatedProduct.price;
        it->stock = updatedProduct.stock;
        it->category = updatedProduct.category;
        it->updatedAt = getCurrentTimestamp();
        
        return *it;
    }
    
    // Delete a product
    bool deleteProduct(int id) {
        auto it = findProductById(id);
        if (it == products.end()) {
            return false;
        }
        
        products.erase(it);
        return true;
    }
    
private:
    // Get current timestamp in ISO format
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&now_c), "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    }
};
```

## Step 3: Creating Middleware

Next, let's create authentication and validation middleware:

```cpp
// src/middleware/auth_middleware.hpp
#pragma once
#include <boson/boson.hpp>
#include <string>
#include <vector>
#include <unordered_map>

class AuthMiddleware {
public:
    // Simple token validation (in a real app, use JWT or other secure tokens)
    static boson::MiddlewareFunction authenticate() {
        return [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
            std::string token = req.header("Authorization");
            
            // Check if Authorization header exists and has correct format
            if (token.empty() || !token.starts_with("Bearer ")) {
                res.status(401).jsonObject({
                    {"error", "Unauthorized"},
                    {"message", "Authentication token is missing or invalid"}
                });
                return;
            }
            
            // Extract token
            std::string actualToken = token.substr(7);  // Remove "Bearer " prefix
            
            // Validate token (simplified for example)
            try {
                // In a real app, verify JWT signature, check expiry, etc.
                auto user = validateToken(actualToken);
                
                // Store user information in request for later use
                req.setContext("user", user);
                next();
            } 
            catch (const std::exception& e) {
                res.status(401).jsonObject({
                    {"error", "Unauthorized"},
                    {"message", e.what()}
                });
            }
        };
    }
    
    // Role-based access control
    static boson::MiddlewareFunction requireRole(const std::vector<std::string>& roles) {
        return [roles](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
            // Get user from context (set by authenticate middleware)
            auto userPtr = req.getContext<nlohmann::json*>("user");
            
            if (!userPtr) {
                res.status(401).jsonObject({
                    {"error", "Unauthorized"},
                    {"message", "Authentication required"}
                });
                return;
            }
            
            // Check if user has required role
            bool hasRequiredRole = false;
            if (userPtr->contains("roles") && (*userPtr)["roles"].is_array()) {
                auto userRoles = (*userPtr)["roles"].get<std::vector<std::string>>();
                
                for (const auto& role : roles) {
                    if (std::find(userRoles.begin(), userRoles.end(), role) != userRoles.end()) {
                        hasRequiredRole = true;
                        break;
                    }
                }
            }
            
            if (!hasRequiredRole) {
                res.status(403).jsonObject({
                    {"error", "Forbidden"},
                    {"message", "Insufficient permissions"}
                });
                return;
            }
            
            next();
        };
    }
    
private:
    // In a real application, this would validate JWT tokens
    static nlohmann::json* validateToken(const std::string& token) {
        static std::unordered_map<std::string, nlohmann::json> tokenStore = {
            {"user_token", {
                {"id", 2},
                {"name", "Regular User"},
                {"email", "user@example.com"},
                {"roles", {"user"}}
            }},
            {"admin_token", {
                {"id", 1},
                {"name", "Admin User"},
                {"email", "admin@example.com"},
                {"roles", {"admin", "user"}}
            }}
        };
        
        // Check if token exists
        auto it = tokenStore.find(token);
        if (it == tokenStore.end()) {
            throw std::runtime_error("Invalid token");
        }
        
        return &it->second;
    }
};

// src/middleware/validation_middleware.hpp
#pragma once
#include <boson/boson.hpp>
#include <functional>
#include <map>
#include <string>

class ValidationMiddleware {
public:
    // Validate request body against a validation function
    static boson::MiddlewareFunction validateBody(
        std::function<std::map<std::string, std::string>(const nlohmann::json&)> validator) {
        
        return [validator](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
            try {
                // Parse request body as JSON
                nlohmann::json body = req.json();
                
                // Validate using the provided validator
                auto errors = validator(body);
                
                // If validation fails, return error response
                if (!errors.empty()) {
                    res.status(400).jsonObject({
                        {"error", "Validation failed"},
                        {"validation_errors", errors}
                    });
                    return;
                }
                
                // Store validated body in request context
                req.setContext("validatedBody", new nlohmann::json(body));
                next();
            } 
            catch (const nlohmann::json::exception& e) {
                // Handle JSON parsing errors
                res.status(400).jsonObject({
                    {"error", "Invalid JSON"},
                    {"message", e.what()}
                });
            }
            catch (const std::exception& e) {
                // Handle other exceptions
                res.status(500).jsonObject({
                    {"error", "Validation error"},
                    {"message", e.what()}
                });
            }
        };
    }
};
```

## Step 4: Creating Controllers

Now, let's create controllers to handle routes:

```cpp
// src/controllers/user_controller.hpp
#pragma once
#include <boson/boson.hpp>
#include <boson/controller.hpp>
#include <memory>
#include "../services/user_service.hpp"
#include "../models/user.hpp"
#include "../middleware/auth_middleware.hpp"
#include "../middleware/validation_middleware.hpp"

class UserController : public boson::Controller {
private:
    UserService& userService;
    
public:
    UserController(UserService& service) : userService(service) {}
    
    std::string basePath() const override {
        return "/api/users";
    }
    
    // Controller middleware - applied to all routes
    std::vector<boson::MiddlewareFunction> middleware() const override {
        return {
            // Log all requests to user endpoints
            [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
                std::cout << "Request to User API: " << req.method() << " " << req.path() << std::endl;
                next();
            }
        };
    }
    
    // GET /api/users - Get all users
    void getUsers(const boson::Request& req, boson::Response& res) {
        try {
            auto users = userService.getAllUsers();
            
            // Convert users to JSON array
            nlohmann::json usersJson = nlohmann::json::array();
            for (const auto& user : users) {
                usersJson.push_back(user.toJson());
            }
            
            res.jsonObject({
                {"users", usersJson},
                {"count", users.size()}
            });
        }
        catch (const std::exception& e) {
            res.status(500).jsonObject({
                {"error", "Failed to fetch users"},
                {"message", e.what()}
            });
        }
    }
    
    // GET /api/users/:id - Get user by ID
    void getUserById(const boson::Request& req, boson::Response& res) {
        try {
            int id = std::stoi(req.param("id"));
            auto user = userService.getUserById(id);
            
            if (!user) {
                res.status(404).jsonObject({
                    {"error", "User not found"},
                    {"id", id}
                });
                return;
            }
            
            res.jsonObject(user->toJson());
        }
        catch (const std::invalid_argument& e) {
            res.status(400).jsonObject({
                {"error", "Invalid user ID"},
                {"message", "User ID must be a number"}
            });
        }
        catch (const std::exception& e) {
            res.status(500).jsonObject({
                {"error", "Failed to fetch user"},
                {"message", e.what()}
            });
        }
    }
    
    // POST /api/users - Create a new user
    void createUser(const boson::Request& req, boson::Response& res) {
        try {
            // Get validated body from context
            auto bodyPtr = req.getContext<nlohmann::json*>("validatedBody");
            if (!bodyPtr) {
                throw std::runtime_error("Request body not validated");
            }
            
            // Create user from JSON
            User user = User::fromJson(*bodyPtr);
            
            // Set initial password hash
            if (bodyPtr->contains("password")) {
                // In a real app, hash the password
                user.passwordHash = (*bodyPtr)["password"];
            }
            
            // Create user in service
            User createdUser = userService.createUser(user);
            
            res.status(201).jsonObject({
                {"message", "User created successfully"},
                {"user", createdUser.toJson()}
            });
        }
        catch (const std::exception& e) {
            res.status(500).jsonObject({
                {"error", "Failed to create user"},
                {"message", e.what()}
            });
        }
    }
    
    // PUT /api/users/:id - Update a user
    void updateUser(const boson::Request& req, boson::Response& res) {
        try {
            int id = std::stoi(req.param("id"));
            
            // Get validated body from context
            auto bodyPtr = req.getContext<nlohmann::json*>("validatedBody");
            if (!bodyPtr) {
                throw std::runtime_error("Request body not validated");
            }
            
            // Check if user exists
            auto existingUser = userService.getUserById(id);
            if (!existingUser) {
                res.status(404).jsonObject({
                    {"error", "User not found"},
                    {"id", id}
                });
                return;
            }
            
            // Update user with new data
            User updatedUser = User::fromJson(*bodyPtr);
            updatedUser.id = id;
            
            // If password is provided, hash it
            if (bodyPtr->contains("password")) {
                // In a real app, hash the password
                updatedUser.passwordHash = (*bodyPtr)["password"];
            }
            
            // Update user in service
            auto result = userService.updateUser(id, updatedUser);
            
            res.jsonObject({
                {"message", "User updated successfully"},
                {"user", result->toJson()}
            });
        }
        catch (const std::invalid_argument& e) {
            res.status(400).jsonObject({
                {"error", "Invalid user ID"},
                {"message", "User ID must be a number"}
            });
        }
        catch (const std::exception& e) {
            res.status(500).jsonObject({
                {"error", "Failed to update user"},
                {"message", e.what()}
            });
        }
    }
    
    // DELETE /api/users/:id - Delete a user
    void deleteUser(const boson::Request& req, boson::Response& res) {
        try {
            int id = std::stoi(req.param("id"));
            
            bool success = userService.deleteUser(id);
            if (!success) {
                res.status(404).jsonObject({
                    {"error", "User not found"},
                    {"id", id}
                });
                return;
            }
            
            res.jsonObject({
                {"message", "User deleted successfully"},
                {"id", id}
            });
        }
        catch (const std::invalid_argument& e) {
            res.status(400).jsonObject({
                {"error", "Invalid user ID"},
                {"message", "User ID must be a number"}
            });
        }
        catch (const std::exception& e) {
            res.status(500).jsonObject({
                {"error", "Failed to delete user"},
                {"message", e.what()}
            });
        }
    }
    
    // POST /api/users/login - User login
    void login(const boson::Request& req, boson::Response& res) {
        try {
            nlohmann::json body = req.json();
            
            if (!body.contains("email") || !body.contains("password")) {
                res.status(400).jsonObject({
                    {"error", "Missing credentials"},
                    {"message", "Email and password are required"}
                });
                return;
            }
            
            std::string email = body["email"];
            std::string password = body["password"];
            
            auto user = userService.authenticate(email, password);
            if (!user) {
                res.status(401).jsonObject({
                    {"error", "Authentication failed"},
                    {"message", "Invalid email or password"}
                });
                return;
            }
            
            // In a real app, generate a JWT token
            std::string token = user->roles.size() > 1 ? "admin_token" : "user_token";
            
            res.jsonObject({
                {"message", "Login successful"},
                {"token", token},
                {"user", user->toJson()}
            });
        }
        catch (const std::exception& e) {
            res.status(500).jsonObject({
                {"error", "Login failed"},
                {"message", e.what()}
            });
        }
    }
};

// src/controllers/product_controller.hpp
#pragma once
#include <boson/boson.hpp>
#include <boson/controller.hpp>
#include <memory>
#include <string>
#include "../services/product_service.hpp"
#include "../models/product.hpp"
#include "../middleware/auth_middleware.hpp"
#include "../middleware/validation_middleware.hpp"

class ProductController : public boson::Controller {
private:
    ProductService& productService;
    
public:
    ProductController(ProductService& service) : productService(service) {}
    
    std::string basePath() const override {
        return "/api/products";
    }
    
    // GET /api/products - Get all products
    void getProducts(const boson::Request& req, boson::Response& res) {
        try {
            std::vector<Product> products;
            
            // Check if category filter is provided
            std::string category = req.query("category");
            if (!category.empty()) {
                products = productService.getProductsByCategory(category);
            } else {
                products = productService.getAllProducts();
            }
            
            // Convert products to JSON array
            nlohmann::json productsJson = nlohmann::json::array();
            for (const auto& product : products) {
                productsJson.push_back(product.toJson());
            }
            
            res.jsonObject({
                {"products", productsJson},
                {"count", products.size()}
            });
        }
        catch (const std::exception& e) {
            res.status(500).jsonObject({
                {"error", "Failed to fetch products"},
                {"message", e.what()}
            });
        }
    }
    
    // GET /api/products/:id - Get product by ID
    void getProductById(const boson::Request& req, boson::Response& res) {
        try {
            int id = std::stoi(req.param("id"));
            auto product = productService.getProductById(id);
            
            if (!product) {
                res.status(404).jsonObject({
                    {"error", "Product not found"},
                    {"id", id}
                });
                return;
            }
            
            res.jsonObject(product->toJson());
        }
        catch (const std::invalid_argument& e) {
            res.status(400).jsonObject({
                {"error", "Invalid product ID"},
                {"message", "Product ID must be a number"}
            });
        }
        catch (const std::exception& e) {
            res.status(500).jsonObject({
                {"error", "Failed to fetch product"},
                {"message", e.what()}
            });
        }
    }
    
    // POST /api/products - Create a new product
    void createProduct(const boson::Request& req, boson::Response& res) {
        try {
            // Get validated body from context
            auto bodyPtr = req.getContext<nlohmann::json*>("validatedBody");
            if (!bodyPtr) {
                throw std::runtime_error("Request body not validated");
            }
            
            // Create product from JSON
            Product product = Product::fromJson(*bodyPtr);
            
            // Create product in service
            Product createdProduct = productService.createProduct(product);
            
            res.status(201).jsonObject({
                {"message", "Product created successfully"},
                {"product", createdProduct.toJson()}
            });
        }
        catch (const std::exception& e) {
            res.status(500).jsonObject({
                {"error", "Failed to create product"},
                {"message", e.what()}
            });
        }
    }
    
    // PUT /api/products/:id - Update a product
    void updateProduct(const boson::Request& req, boson::Response& res) {
        try {
            int id = std::stoi(req.param("id"));
            
            // Get validated body from context
            auto bodyPtr = req.getContext<nlohmann::json*>("validatedBody");
            if (!bodyPtr) {
                throw std::runtime_error("Request body not validated");
            }
            
            // Check if product exists
            auto existingProduct = productService.getProductById(id);
            if (!existingProduct) {
                res.status(404).jsonObject({
                    {"error", "Product not found"},
                    {"id", id}
                });
                return;
            }
            
            // Update product with new data
            Product updatedProduct = Product::fromJson(*bodyPtr);
            updatedProduct.id = id;
            
            // Update product in service
            auto result = productService.updateProduct(id, updatedProduct);
            
            res.jsonObject({
                {"message", "Product updated successfully"},
                {"product", result->toJson()}
            });
        }
        catch (const std::invalid_argument& e) {
            res.status(400).jsonObject({
                {"error", "Invalid product ID"},
                {"message", "Product ID must be a number"}
            });
        }
        catch (const std::exception& e) {
            res.status(500).jsonObject({
                {"error", "Failed to update product"},
                {"message", e.what()}
            });
        }
    }
    
    // DELETE /api/products/:id - Delete a product
    void deleteProduct(const boson::Request& req, boson::Response& res) {
        try {
            int id = std::stoi(req.param("id"));
            
            bool success = productService.deleteProduct(id);
            if (!success) {
                res.status(404).jsonObject({
                    {"error", "Product not found"},
                    {"id", id}
                });
                return;
            }
            
            res.jsonObject({
                {"message", "Product deleted successfully"},
                {"id", id}
            });
        }
        catch (const std::invalid_argument& e) {
            res.status(400).jsonObject({
                {"error", "Invalid product ID"},
                {"message", "Product ID must be a number"}
            });
        }
        catch (const std::exception& e) {
            res.status(500).jsonObject({
                {"error", "Failed to delete product"},
                {"message", e.what()}
            });
        }
    }
};
```

## Step 5: Creating the Main Application

Finally, let's tie everything together in the main application:

```cpp
// src/main.cpp
#include <boson/boson.hpp>
#include <iostream>
#include <string>
#include <memory>
#include <chrono>
#include <stdexcept>

// Include our components
#include "controllers/user_controller.hpp"
#include "controllers/product_controller.hpp"
#include "services/user_service.hpp"
#include "services/product_service.hpp"
#include "middleware/auth_middleware.hpp"
#include "middleware/validation_middleware.hpp"
#include "models/user.hpp"
#include "models/product.hpp"

// Global error handler
auto errorHandler = [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    try {
        next();
    }
    catch (const std::exception& e) {
        // Log the error
        std::cerr << "[ERROR] " << req.method() << " " << req.path() << ": " << e.what() << std::endl;
        
        // Send error response
        res.status(500).jsonObject({
            {"error", "Internal Server Error"},
            {"message", e.what()},
            {"path", req.path()},
            {"timestamp", getCurrentTimestamp()}
        });
    }
};

// Helper for current timestamp
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&now_c), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

int main() {
    try {
        // Initialize Boson framework
        boson::initialize();
        
        // Create service instances
        UserService userService;
        ProductService productService;
        
        // Create server instance
        boson::Server app;
        
        // Add global middleware
        app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
            // Add timestamp to all responses
            auto originalSend = res.getSendCallback();
            res.setSendCallback([originalSend, timestamp = getCurrentTimestamp()](boson::Response& r) {
                // Add headers
                r.header("X-API-Time", timestamp);
                r.header("X-API-Version", "1.0");
                
                // Call original send callback
                originalSend(r);
            });
            
            // Log request
            std::cout << "[" << timestamp << "] " << req.method() << " " << req.path() << std::endl;
            next();
        });
        
        // Add CORS middleware
        app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
            res.header("Access-Control-Allow-Origin", "*");
            res.header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept, Authorization");
            
            if (req.method() == "OPTIONS") {
                return res.status(200).send();
            }
            
            next();
        });
        
        // Add error handler middleware
        app.use(errorHandler);
        
        // Create controllers
        auto userController = std::make_shared<UserController>(userService);
        auto productController = std::make_shared<ProductController>(productService);
        
        // Set up user routes
        auto userRouter = boson::createRouter(userController);
        userRouter.get("/", &UserController::getUsers);
        userRouter.get("/:id", &UserController::getUserById);
        userRouter.post("/", ValidationMiddleware::validateBody(User::validate), &UserController::createUser);
        userRouter.put("/:id", AuthMiddleware::authenticate(), ValidationMiddleware::validateBody(User::validate), &UserController::updateUser);
        userRouter.del("/:id", AuthMiddleware::authenticate(), AuthMiddleware::requireRole({"admin"}), &UserController::deleteUser);
        userRouter.post("/login", &UserController::login);
        
        // Set up product routes
        auto productRouter = boson::createRouter(productController);
        productRouter.get("/", &ProductController::getProducts);
        productRouter.get("/:id", &ProductController::getProductById);
        productRouter.post("/", AuthMiddleware::authenticate(), ValidationMiddleware::validateBody(Product::validate), &ProductController::createProduct);
        productRouter.put("/:id", AuthMiddleware::authenticate(), ValidationMiddleware::validateBody(Product::validate), &ProductController::updateProduct);
        productRouter.del("/:id", AuthMiddleware::authenticate(), AuthMiddleware::requireRole({"admin"}), &ProductController::deleteProduct);
        
        // Mount controllers to the server
        userRouter.mountOn(&app);
        productRouter.mountOn(&app);
        
        // Add root route
        app.get("/", [](const boson::Request& req, boson::Response& res) {
            res.jsonObject({
                {"message", "Welcome to the Boson API Server"},
                {"version", "1.0.0"},
                {"documentation", "/api-docs"}
            });
        });
        
        // Add API documentation route
        app.get("/api-docs", [](const boson::Request& req, boson::Response& res) {
            res.jsonObject({
                {"openapi", "3.0.0"},
                {"info", {
                    {"title", "Boson API Example"},
                    {"version", "1.0.0"},
                    {"description", "A sample API built with Boson Framework"}
                }},
                {"paths", {
                    {"/api/users", {
                        {"get", {
                            {"summary", "Get all users"},
                            {"responses", {{"200", {{"description", "List of users"}}}}}
                        }},
                        {"post", {
                            {"summary", "Create a new user"},
                            {"responses", {{"201", {{"description", "User created"}}}}}
                        }}
                    }},
                    // ... other paths would be defined here ...
                }}
            });
        });
        
        // Configure and start the server
        app.configure(3000, "127.0.0.1");
        std::cout << "REST API server running at http://127.0.0.1:3000" << std::endl;
        return app.listen();
    }
    catch (const std::exception& e) {
        std::cerr << "Server initialization failed: " << e.what() << std::endl;
        return 1;
    }
}
```

## Step 6: Creating the CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.14)
project(boson-rest-api VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boson package
find_package(Boson REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Collect source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link against Boson
target_link_libraries(${PROJECT_NAME} PRIVATE Boson::Boson)

# Copy config directory to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config DESTINATION ${CMAKE_BINARY_DIR})
```

## Testing the API

Once you've built and run your API, you can test it with curl:

```bash
# Get all users
curl http://localhost:3000/api/users

# Login
curl -X POST http://localhost:3000/api/users/login \
  -H "Content-Type: application/json" \
  -d '{"email": "admin@example.com", "password": "hashed_password_here"}'

# Create a new product (with authentication)
curl -X POST http://localhost:3000/api/products \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer admin_token" \
  -d '{"name": "New Product", "description": "A brand new product", "price": 39.99, "stock": 20, "category": "Electronics"}'

# Delete a user (requires admin role)
curl -X DELETE http://localhost:3000/api/users/2 \
  -H "Authorization: Bearer admin_token"
```

## Summary

This example demonstrates how to build a complete REST API using the Boson framework with:

1. **Proper Organization**: Models, Services, Controllers
2. **Authentication**: Token-based authentication middleware
3. **Authorization**: Role-based access control
4. **Validation**: Request validation middleware
5. **Error Handling**: Global error handling
6. **Middleware**: Logging, CORS, and custom middleware
7. **Clean Routing**: Controller-based routing

The architecture follows best practices for creating maintainable and scalable applications:

- **Separation of Concerns**: Each component has a clear responsibility
- **Dependency Injection**: Services are injected into controllers
- **Validation**: Input validation before processing
- **Error Handling**: Consistent error responses
- **Authentication**: Proper authentication and authorization
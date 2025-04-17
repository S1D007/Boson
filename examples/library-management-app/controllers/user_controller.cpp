#include "user_controller.hpp"
#include "../models/user.hpp"
#include "../models/loan.hpp"
#include "../models/book.hpp"
#include <stdexcept>
#include <string>
#include <vector>

std::string UserController::basePath() const {
    return "/users";
}

void UserController::getAllUsers(const boson::Request& req, boson::Response& res) {
    auto& userRepo = UserRepository::getInstance();
    auto users = userRepo.getAllUsers();
    
    nlohmann::json usersJson = nlohmann::json::array();
    
    for (const auto& user : users) {
        usersJson.push_back(user.toJson());
    }
    
    res.status(200).jsonObject({
        {"users", usersJson},
        {"total", users.size()}
    });
}

void UserController::getUserById(const boson::Request& req, boson::Response& res) {
    std::string idStr = req.param("id");
    int id;
    
    try {
        id = std::stoi(idStr);
    } catch (const std::exception&) {
        res.status(400).jsonObject({
            {"error", "Invalid user ID format"}
        });
        return;
    }
    
    auto& userRepo = UserRepository::getInstance();
    const User* user = userRepo.getUserById(id);
    
    if (!user) {
        res.status(404).jsonObject({
            {"error", "User not found"},
            {"id", id}
        });
        return;
    }
    
    res.status(200).jsonObject(user->toJson());
}

void UserController::createUser(const boson::Request& req, boson::Response& res) {
    try {
        auto json = req.json();
        
        if (!json.contains("name") || !json.contains("email") || !json.contains("password")) {
            res.status(400).jsonObject({
                {"error", "Missing required fields"},
                {"required", {"name", "email", "password"}}
            });
            return;
        }
        
        User::Role role = User::Role::MEMBER;
        if (json.contains("role")) {
            std::string roleStr = json["role"];
            if (roleStr == "LIBRARIAN") role = User::Role::LIBRARIAN;
            else if (roleStr == "ADMIN") role = User::Role::ADMIN;
        }
        
        User newUser(
            0, // Temporary ID, will be replaced
            json["name"],
            json["email"],
            json["password"],
            role
        );
        
        auto& userRepo = UserRepository::getInstance();
        
        if (userRepo.getUserByEmail(newUser.getEmail())) {
            res.status(400).jsonObject({
                {"error", "Email already in use"}
            });
            return;
        }
        
        User createdUser = userRepo.addUser(newUser);
        
        res.status(201).jsonObject({
            {"message", "User created successfully"},
            {"user", createdUser.toJson()}
        });
    } catch (const std::exception& e) {
        res.status(400).jsonObject({
            {"error", "Failed to create user"},
            {"message", e.what()}
        });
    }
}

void UserController::updateUser(const boson::Request& req, boson::Response& res) {
    std::string idStr = req.param("id");
    int id;
    
    try {
        id = std::stoi(idStr);
    } catch (const std::exception&) {
        res.status(400).jsonObject({
            {"error", "Invalid user ID format"}
        });
        return;
    }
    
    auto& userRepo = UserRepository::getInstance();
    User* user = userRepo.getUserById(id);
    
    if (!user) {
        res.status(404).jsonObject({
            {"error", "User not found"},
            {"id", id}
        });
        return;
    }
    
    try {
        auto json = req.json();
        
        User::Role role = user->getRole();
        if (json.contains("role")) {
            std::string roleStr = json["role"];
            if (roleStr == "MEMBER") role = User::Role::MEMBER;
            else if (roleStr == "LIBRARIAN") role = User::Role::LIBRARIAN;
            else if (roleStr == "ADMIN") role = User::Role::ADMIN;
        }
        
        User updatedUser(
            id,
            json.contains("name") ? json["name"].get<std::string>() : user->getName(),
            json.contains("email") ? json["email"].get<std::string>() : user->getEmail(),
            json.contains("password") ? json["password"].get<std::string>() : "", // Password only updated if provided
            role
        );
        
        if (userRepo.updateUser(updatedUser)) {
            res.status(200).jsonObject({
                {"message", "User updated successfully"},
                {"user", updatedUser.toJson()}
            });
        } else {
            res.status(500).jsonObject({
                {"error", "Failed to update user"}
            });
        }
    } catch (const std::exception& e) {
        res.status(400).jsonObject({
            {"error", "Invalid request data"},
            {"message", e.what()}
        });
    }
}

void UserController::deleteUser(const boson::Request& req, boson::Response& res) {
    std::string idStr = req.param("id");
    int id;
    
    try {
        id = std::stoi(idStr);
    } catch (const std::exception&) {
        res.status(400).jsonObject({
            {"error", "Invalid user ID format"}
        });
        return;
    }
    
    auto& userRepo = UserRepository::getInstance();
    const User* user = userRepo.getUserById(id);
    
    if (!user) {
        res.status(404).jsonObject({
            {"error", "User not found"},
            {"id", id}
        });
        return;
    }
    
    if (userRepo.deleteUser(id)) {
        res.status(200).jsonObject({
            {"message", "User deleted successfully"},
            {"id", id}
        });
    } else {
        res.status(500).jsonObject({
            {"error", "Failed to delete user"}
        });
    }
}

void UserController::login(const boson::Request& req, boson::Response& res) {
    try {
        auto json = req.json();
        
        if (!json.contains("email") || !json.contains("password")) {
            res.status(400).jsonObject({
                {"error", "Missing required fields"},
                {"required", {"email", "password"}}
            });
            return;
        }
        
        std::string email = json["email"];
        std::string password = json["password"];
        
        auto& userRepo = UserRepository::getInstance();
        User* user = userRepo.authenticateUser(email, password);
        
        if (!user) {
            res.status(401).jsonObject({
                {"error", "Invalid email or password"}
            });
            return;
        }
        
        res.status(200).jsonObject({
            {"message", "Login successful"},
            {"user", user->toJson()}
        });
    } catch (const std::exception& e) {
        res.status(400).jsonObject({
            {"error", "Login failed"},
            {"message", e.what()}
        });
    }
}

void UserController::getUserLoans(const boson::Request& req, boson::Response& res) {
    std::string idStr = req.param("id");
    int id;
    
    try {
        id = std::stoi(idStr);
    } catch (const std::exception&) {
        res.status(400).jsonObject({
            {"error", "Invalid user ID format"}
        });
        return;
    }
    
    auto& userRepo = UserRepository::getInstance();
    const User* user = userRepo.getUserById(id);
    
    if (!user) {
        res.status(404).jsonObject({
            {"error", "User not found"},
            {"id", id}
        });
        return;
    }
    
    auto& loanRepo = LoanRepository::getInstance();
    auto loans = loanRepo.getLoansByUserId(id);
    auto& bookRepo = BookRepository::getInstance();
    
    nlohmann::json loansJson = nlohmann::json::array();
    
    for (const auto& loan : loans) {
        auto loanJson = loan.toJson();
        
        const Book* book = bookRepo.getBookById(loan.getBookId());
        if (book) {
            loanJson["book"] = book->toJson();
        }
        
        loansJson.push_back(loanJson);
    }
    
    res.status(200).jsonObject({
        {"loans", loansJson},
        {"total", loans.size()}
    });
}
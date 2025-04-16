#pragma once

#include <boson/controller.hpp>
#include <boson/request.hpp>
#include <boson/response.hpp>
#include <string>
#include <vector>
#include "../services/user_service.hpp"

class ApiController : public boson::Controller {
public:
    ApiController() : userService(std::make_shared<UserService>()) {}
    virtual ~ApiController() = default;

    std::string basePath() const override { 
        return "/api"; 
    }

    // List all users
    void getUsers(const boson::Request& req, boson::Response& res) {
        auto users = userService->getAllUsers();
        
        // Create JSON array of users
        nlohmann::json usersArray = nlohmann::json::array();
        for (const auto& user : users) {
            usersArray.push_back(user.toJson());
        }
        
        // Directly use jsonObject with initializer list
        res.jsonObject({
            {"users", usersArray},
            {"count", users.size()}
        });
    }

    // Get user by ID
    void getUserById(const boson::Request& req, boson::Response& res) {
        std::string id = req.param("id");
        
        try {
            int userId = std::stoi(id);
            auto user = userService->getUserById(userId);
            
            if (user.id == 0) { // User not found
                res.status(404).jsonObject({
                    {"error", "User not found"},
                    {"id", id}
                });
                return;
            }
            
            res.jsonObject({
                {"user", user.toJson()}
            });
        } catch (const std::exception& e) {
            res.status(400).jsonObject({
                {"error", "Invalid user ID"},
                {"message", e.what()}
            });
        }
    }

    // Create a new user
    void createUser(const boson::Request& req, boson::Response& res) {
        try {
            auto requestBody = req.json();
            
            if (!requestBody.contains("name") || !requestBody.contains("email")) {
                res.status(400).jsonObject({
                    {"error", "Missing required fields"},
                    {"required", {"name", "email"}}
                });
                return;
            }
            
            User newUser;
            newUser.name = requestBody["name"];
            newUser.email = requestBody["email"];
            
            auto createdUser = userService->createUser(newUser);
            
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

    // Update user
    void updateUser(const boson::Request& req, boson::Response& res) {
        std::string id = req.param("id");
        
        try {
            int userId = std::stoi(id);
            auto requestBody = req.json();
            
            User updatedUser;
            updatedUser.id = userId;
            
            if (requestBody.contains("name")) {
                updatedUser.name = requestBody["name"];
            }
            
            if (requestBody.contains("email")) {
                updatedUser.email = requestBody["email"];
            }
            
            auto result = userService->updateUser(updatedUser);
            
            if (!result) {
                res.status(404).jsonObject({
                    {"error", "User not found"},
                    {"id", id}
                });
                return;
            }
            
            res.jsonObject({
                {"message", "User updated successfully"},
                {"user", updatedUser.toJson()}
            });
        } catch (const std::exception& e) {
            res.status(400).jsonObject({
                {"error", "Failed to update user"},
                {"message", e.what()}
            });
        }
    }

    // Delete user
    void deleteUser(const boson::Request& req, boson::Response& res) {
        std::string id = req.param("id");
        
        try {
            int userId = std::stoi(id);
            bool result = userService->deleteUser(userId);
            
            if (!result) {
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
        } catch (const std::exception& e) {
            res.status(400).jsonObject({
                {"error", "Failed to delete user"},
                {"message", e.what()}
            });
        }
    }

private:
    std::shared_ptr<UserService> userService;
};
#pragma once

#include <boson/controller.hpp>
#include <boson/request.hpp>
#include <boson/response.hpp>
#include <string>
#include "../services/user_service.hpp"
#include "../models/user.hpp"

/**
 * @class ApiController
 * @brief Controller for API routes
 */
class ApiController : public boson::Controller {
public:
    /**
     * @brief Constructor
     */
    ApiController() {
        userService_ = std::make_shared<UserService>();
    }

    /**
     * @brief Get the base path for this controller
     * @return The base path
     */
    std::string basePath() const override {
        return "/api";
    }
    
    /**
     * @brief Register routes with the router
     */
    void registerRoutes() override {
        
        GET("/users", &ApiController::getUsers);
        GET("/users/{id}", &ApiController::getUserById);
        POST("/users", &ApiController::createUser);
        PUT("/users/{id}", &ApiController::updateUser);
        DELETE("/users/{id}", &ApiController::deleteUser);
    }

    /**
     * @brief Get all users
     * @param request The HTTP request
     * @return The HTTP response
     */
    boson::Response getUsers(const boson::Request& request) {
        
        auto users = userService_->getAllUsers();
        
        
        nlohmann::json usersJson = nlohmann::json::array();
        for (const auto& user : users) {
            usersJson.push_back(user.toJson());
        }
        
        return boson::Response::ok().json({{"users", usersJson}});
    }
    
    /**
     * @brief Get user by ID
     * @param request The HTTP request
     * @return The HTTP response
     */
    boson::Response getUserById(const boson::Request& request) {
        std::string id = request.param("id");
        
        
        auto userOpt = userService_->getUserById(id);
        if (!userOpt) {
            return boson::Response::notFound().json({{"error", "User not found"}});
        }
        
        return boson::Response::ok().json(userOpt->toJson());
    }
    
    /**
     * @brief Create a new user
     * @param request The HTTP request
     * @return The HTTP response
     */
    boson::Response createUser(const boson::Request& request) {
        try {
            
            auto json = request.json();
            
            
            User user = User::fromJson(json);
            
            
            user = userService_->createUser(user);
            
            return boson::Response::created().json(user.toJson());
        } catch (const std::exception& e) {
            return boson::Response::badRequest().json({{"error", e.what()}});
        }
    }
    
    /**
     * @brief Update an existing user
     * @param request The HTTP request
     * @return The HTTP response
     */
    boson::Response updateUser(const boson::Request& request) {
        std::string id = request.param("id");
        
        try {
            
            auto json = request.json();
            
            
            User user = User::fromJson(json);
            user.setId(id);
            
            
            auto updatedUser = userService_->updateUser(user);
            if (!updatedUser) {
                return boson::Response::notFound().json({{"error", "User not found"}});
            }
            
            return boson::Response::ok().json(updatedUser->toJson());
        } catch (const std::exception& e) {
            return boson::Response::badRequest().json({{"error", e.what()}});
        }
    }
    
    /**
     * @brief Delete a user
     * @param request The HTTP request
     * @return The HTTP response
     */
    boson::Response deleteUser(const boson::Request& request) {
        std::string id = request.param("id");
        
        
        bool success = userService_->deleteUser(id);
        if (!success) {
            return boson::Response::notFound().json({{"error", "User not found"}});
        }
        
        return boson::Response::ok().json({{"message", "User deleted"}});
    }

private:
    std::shared_ptr<UserService> userService_;
};
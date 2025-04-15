#pragma once

#include <boson/controller.hpp>
#include <boson/request.hpp>
#include <boson/response.hpp>
#include <string>
#include <cstdlib>
#include "../external/json.hpp"

/**
 * @class HelloController
 * @brief A simple controller for demo purposes
 */
class HelloController : public boson::Controller {
public:
    /**
     * @brief Get the base path for this controller
     * @return The base path
     */
    std::string basePath() const override {
        return "/";
    }
    
    /**
     * @brief Register routes with the router
     */
    void registerRoutes() override {
        GET("/", &HelloController::index);
        GET("/hello", &HelloController::hello);
        GET("/hello/json", &HelloController::helloJson);
        GET("/hello/{name}", &HelloController::helloName);
        GET("/error", &HelloController::triggerError);
    }

    /**
     * @brief Index route handler
     * @param request The HTTP request
     * @return The HTTP response
     */
    boson::Response index(const boson::Request& request) {
        return boson::Response::ok().text("Welcome to Boson Framework!");
    }
    
    /**
     * @brief Simple hello route
     * @param request The HTTP request
     * @return The HTTP response
     */
    boson::Response hello(const boson::Request& request) {
        return boson::Response::ok().text("Hello, World!");
    }
    
    /**
     * @brief JSON response example
     * @param request The HTTP request
     * @return The HTTP response
     */
    boson::Response helloJson(const boson::Request& request) {
        // Create JSON object without using initializer list syntax with commas
        nlohmann::json response = nlohmann::json::object();
        response["message"] = "Hello, World!";
        response["framework"] = "Boson";
        response["timestamp"] = std::time(nullptr);
        
        return boson::Response::ok().json(response);
    }
    
    /**
     * @brief Route with path parameter
     * @param request The HTTP request
     * @return The HTTP response
     */
    boson::Response helloName(const boson::Request& request) {
        std::string name = request.param("name");
        return boson::Response::ok().text("Hello, " + name + "!");
    }
    
    /**
     * @brief Error demonstration route
     * @param request The HTTP request
     * @return The HTTP response
     */
    boson::Response triggerError(const boson::Request& request) {
        int errorType = std::rand() % 3;
        
        // Create JSON objects without using initializer list syntax
        nlohmann::json errorJson = nlohmann::json::object();
        
        switch (errorType) {
            case 0:
                errorJson["error"] = "Resource not found";
                return boson::Response::notFound().json(errorJson);
            case 1:
                errorJson["error"] = "Internal server error";
                return boson::Response::serverError().json(errorJson);
            default:
                errorJson["error"] = "Access forbidden";
                return boson::Response::forbidden().json(errorJson);
        }
    }
};
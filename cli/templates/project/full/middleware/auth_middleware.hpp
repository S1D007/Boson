#pragma once

#include <boson/middleware.hpp>
#include <boson/request.hpp>
#include <boson/response.hpp>
#include <external/json.hpp>
#include <string>
#include <functional>

class AuthMiddleware : public boson::Middleware {
public:
    AuthMiddleware() = default;
    ~AuthMiddleware() = default;
    
    void process(const boson::Request& req, boson::Response& res, std::function<void()>& next) {
        // Check for authentication token in the request headers
        std::string authHeader = req.header("Authorization");
        
        if (authHeader.empty() || !validateToken(authHeader)) {
            // If no token or invalid token, return 401 Unauthorized using direct JSON initialization
            res.status(401).jsonObject({
                {"error", "Unauthorized"},
                {"message", "Valid authentication token required"}
            });
            return;
        }
        
        // If token is valid, continue to the next middleware or route handler
        next();
    }
    
private:
    // Simple token validation (this should be more secure in a real application)
    bool validateToken(const std::string& authHeader) {
        // Check if the header starts with "Bearer "
        if (authHeader.size() < 7 || authHeader.substr(0, 7) != "Bearer ") {
            return false;
        }
        
        // Extract the token
        std::string token = authHeader.substr(7);
        
        // In a real application, you would validate the token properly
        // For this example, we just check if it's not empty
        return !token.empty();
    }
};
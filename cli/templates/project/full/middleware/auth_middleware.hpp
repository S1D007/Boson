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
        std::string authHeader = req.header("Authorization");
        
        if (authHeader.empty() || !validateToken(authHeader)) {
            res.status(401).jsonObject({
                {"error", "Unauthorized"},
                {"message", "Valid authentication token required"}
            });
            return;
        }
        
        next();
    }
    
private:
    bool validateToken(const std::string& authHeader) {
        if (authHeader.size() < 7 || authHeader.substr(0, 7) != "Bearer ") {
            return false;
        }
        
        std::string token = authHeader.substr(7);
        
        return !token.empty();
    }
};
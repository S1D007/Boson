#pragma once

#include <boson/boson.hpp>
#include <string>
#include <vector>
#include <stdexcept>

// User model class
struct User {
    int id = 0;
    std::string name;
    std::string email;
    bool active = true;
    
    // Convert to JSON using direct initialization
    nlohmann::json toJson() const {
        return {
            {"id", id},
            {"name", name},
            {"email", email},
            {"active", active}
        };
    }
    
    // Validate user data
    static bool validate(const nlohmann::json& j) {
        if (!j.contains("name") || !j["name"].is_string() || j["name"].get<std::string>().empty()) {
            throw std::invalid_argument("Name is required and must be a non-empty string");
        }
        
        if (!j.contains("email") || !j["email"].is_string() || j["email"].get<std::string>().empty()) {
            throw std::invalid_argument("Email is required and must be a non-empty string");
        }
        
        // Simple email validation (contains @ character)
        if (j["email"].get<std::string>().find('@') == std::string::npos) {
            throw std::invalid_argument("Invalid email format");
        }
        
        return true;
    }
    
    // Create from JSON
    static User fromJson(const nlohmann::json& j) {
        User user;
        if (j.contains("id")) {
            user.id = j["id"];
        }
        user.name = j["name"];
        user.email = j["email"];
        if (j.contains("active")) {
            user.active = j["active"];
        }
        return user;
    }
};

// JSON serialization helper for the User struct
namespace boson {
    inline void to_json(nlohmann::json& j, const User& user) {
        j = user.toJson();
    }
}
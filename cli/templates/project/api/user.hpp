#pragma once

#include <string>
#include <vector>
#include "../external/json.hpp"

/**
 * @class User
 * @brief Represents a user entity
 */
class User {
public:
    /**
     * @brief Default constructor
     */
    User() = default;
    
    /**
     * @brief Constructor with parameters
     */
    User(const std::string& id, const std::string& name, const std::string& email)
        : id_(id), name_(name), email_(email) {}
    
    /**
     * @brief Get the ID
     * @return The ID
     */
    const std::string& id() const { return id_; }
    
    /**
     * @brief Set the ID
     * @param id The new ID
     */
    void setId(const std::string& id) { id_ = id; }
    
    /**
     * @brief Get the name
     * @return The name
     */
    const std::string& name() const { return name_; }
    
    /**
     * @brief Set the name
     * @param name The new name
     */
    void setName(const std::string& name) { name_ = name; }
    
    /**
     * @brief Get the email
     * @return The email
     */
    const std::string& email() const { return email_; }
    
    /**
     * @brief Set the email
     * @param email The new email
     */
    void setEmail(const std::string& email) { email_ = email; }
    
    /**
     * @brief Convert to JSON object
     * @return JSON representation of the user
     */
    nlohmann::json toJson() const {
        return {
            {"id", id_},
            {"name", name_},
            {"email", email_}
        };
    }
    
    /**
     * @brief Create from JSON object
     * @param json The JSON object
     * @return A new User instance
     */
    static User fromJson(const nlohmann::json& json) {
        User user;
        if (json.contains("id")) {
            user.setId(json["id"]);
        }
        if (json.contains("name")) {
            user.setName(json["name"]);
        }
        if (json.contains("email")) {
            user.setEmail(json["email"]);
        }
        return user;
    }
    
private:
    std::string id_;
    std::string name_;
    std::string email_;
};
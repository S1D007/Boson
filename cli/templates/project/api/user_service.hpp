#pragma once

#include <vector>
#include <memory>
#include <string>
#include <optional>
#include <algorithm>
#include "../models/user.hpp"

/**
 * @class UserService
 * @brief Service for user-related operations
 */
class UserService {
public:
    /**
     * @brief Default constructor
     */
    UserService() {
        
        users_.push_back(User("1", "John Doe", "john@example.com"));
        users_.push_back(User("2", "Jane Smith", "jane@example.com"));
        nextId_ = 3;
    }
    
    /**
     * @brief Get all users
     * @return Vector of all users
     */
    std::vector<User> getAllUsers() {
        return users_;
    }
    
    /**
     * @brief Get a user by ID
     * @param id The user ID
     * @return The user if found, std::nullopt otherwise
     */
    std::optional<User> getUserById(const std::string& id) {
        auto it = std::find_if(users_.begin(), users_.end(),
                             [&](const User& user) { return user.id() == id; });
        
        if (it != users_.end()) {
            return *it;
        }
        
        return std::nullopt;
    }
    
    /**
     * @brief Create a new user
     * @param user The user to create (ID will be assigned)
     * @return The created user
     */
    User createUser(User user) {
        
        user.setId(std::to_string(nextId_++));
        
        
        users_.push_back(user);
        
        return user;
    }
    
    /**
     * @brief Update an existing user
     * @param user The user with updated data
     * @return The updated user if found, std::nullopt otherwise
     */
    std::optional<User> updateUser(const User& user) {
        auto it = std::find_if(users_.begin(), users_.end(),
                             [&](const User& u) { return u.id() == user.id(); });
        
        if (it != users_.end()) {
            *it = user;
            return *it;
        }
        
        return std::nullopt;
    }
    
    /**
     * @brief Delete a user
     * @param id The ID of the user to delete
     * @return True if the user was found and deleted, false otherwise
     */
    bool deleteUser(const std::string& id) {
        auto it = std::find_if(users_.begin(), users_.end(),
                             [&](const User& user) { return user.id() == id; });
        
        if (it != users_.end()) {
            users_.erase(it);
            return true;
        }
        
        return false;
    }
    
private:
    std::vector<User> users_;
    int nextId_;
};
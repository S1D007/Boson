#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include "../models/user.hpp"

class UserService {
public:
    UserService() {
        // Initialize with some sample data
        users = {
            {1, "John Doe", "john@example.com", true},
            {2, "Jane Smith", "jane@example.com", true},
            {3, "Bob Johnson", "bob@example.com", false}
        };
        nextId = 4;
    }
    
    ~UserService() = default;
    
    // Get all users
    std::vector<User> getAllUsers() const {
        return users;
    }
    
    // Get user by ID
    User getUserById(int id) const {
        auto it = std::find_if(users.begin(), users.end(), 
            [id](const User& user) { return user.id == id; });
        
        if (it != users.end()) {
            return *it;
        }
        
        // Return empty user if not found
        return User{};
    }
    
    // Create a new user
    User createUser(const User& user) {
        User newUser = user;
        newUser.id = nextId++;
        users.push_back(newUser);
        return newUser;
    }
    
    // Update an existing user
    bool updateUser(const User& updatedUser) {
        auto it = std::find_if(users.begin(), users.end(), 
            [&updatedUser](const User& user) { return user.id == updatedUser.id; });
        
        if (it == users.end()) {
            return false;
        }
        
        // Update fields if provided
        if (!updatedUser.name.empty()) {
            it->name = updatedUser.name;
        }
        
        if (!updatedUser.email.empty()) {
            it->email = updatedUser.email;
        }
        
        return true;
    }
    
    // Delete a user
    bool deleteUser(int id) {
        auto it = std::find_if(users.begin(), users.end(), 
            [id](const User& user) { return user.id == id; });
        
        if (it == users.end()) {
            return false;
        }
        
        users.erase(it);
        return true;
    }
    
private:
    std::vector<User> users;
    int nextId;
};
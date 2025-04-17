#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../include/external/json.hpp"

class User {
public:
    enum class Role {
        MEMBER,
        LIBRARIAN,
        ADMIN
    };

    User() = default;
    User(int id, std::string name, std::string email, std::string password, Role role = Role::MEMBER);
    
    int getId() const;
    const std::string& getName() const;
    const std::string& getEmail() const;
    bool checkPassword(const std::string& password) const;
    Role getRole() const;
    
    nlohmann::json toJson() const;
    static User fromJson(const nlohmann::json& json);
    
private:
    int id;
    std::string name;
    std::string email;
    std::string passwordHash;
    Role role;
    
    static std::string hashPassword(const std::string& password);
};

class UserRepository {
public:
    static UserRepository& getInstance();
    
    std::vector<User> getAllUsers() const;
    User* getUserById(int id);
    const User* getUserById(int id) const;
    User* getUserByEmail(const std::string& email);
    const User* getUserByEmail(const std::string& email) const;
    
    User addUser(const User& user);
    bool updateUser(const User& user);
    bool deleteUser(int id);
    User* authenticateUser(const std::string& email, const std::string& password);
    
private:
    UserRepository();
    std::vector<User> users;
    int nextId = 1;
};
#include "user.hpp"
#include <algorithm>
#include <stdexcept>
#include <functional>

User::User(int id, std::string name, std::string email, std::string password, Role role)
    : id(id), name(std::move(name)), email(std::move(email)), passwordHash(hashPassword(password)), role(role) {}

int User::getId() const { return id; }
const std::string& User::getName() const { return name; }
const std::string& User::getEmail() const { return email; }
User::Role User::getRole() const { return role; }

bool User::checkPassword(const std::string& password) const {
    return passwordHash == hashPassword(password);
}

std::string User::hashPassword(const std::string& password) {
    std::hash<std::string> hasher;
    return std::to_string(hasher(password));
}

nlohmann::json User::toJson() const {
    std::string roleStr;
    switch (role) {
        case Role::MEMBER: roleStr = "MEMBER"; break;
        case Role::LIBRARIAN: roleStr = "LIBRARIAN"; break;
        case Role::ADMIN: roleStr = "ADMIN"; break;
    }
    
    return {
        {"id", id},
        {"name", name},
        {"email", email},
        {"role", roleStr}
    };
}

User User::fromJson(const nlohmann::json& json) {
    Role role = Role::MEMBER;
    if (json.contains("role")) {
        std::string roleStr = json["role"];
        if (roleStr == "LIBRARIAN") role = Role::LIBRARIAN;
        else if (roleStr == "ADMIN") role = Role::ADMIN;
    }
    
    User user(
        json["id"],
        json["name"],
        json["email"],
        json.contains("password") ? json["password"].get<std::string>() : "",
        role
    );
    
    return user;
}

UserRepository::UserRepository() {
    users = {
        {1, "Admin User", "admin@library.com", "admin123", User::Role::ADMIN},
        {2, "Librarian User", "librarian@library.com", "librarian123", User::Role::LIBRARIAN},
        {3, "John Member", "john@example.com", "john123", User::Role::MEMBER},
        {4, "Jane Member", "jane@example.com", "jane123", User::Role::MEMBER}
    };
    nextId = users.size() + 1;
}

UserRepository& UserRepository::getInstance() {
    static UserRepository instance;
    return instance;
}

std::vector<User> UserRepository::getAllUsers() const {
    return users;
}

User* UserRepository::getUserById(int id) {
    auto it = std::find_if(users.begin(), users.end(), [id](const User& user) {
        return user.getId() == id;
    });
    
    return (it != users.end()) ? &(*it) : nullptr;
}

const User* UserRepository::getUserById(int id) const {
    auto it = std::find_if(users.begin(), users.end(), [id](const User& user) {
        return user.getId() == id;
    });
    
    return (it != users.end()) ? &(*it) : nullptr;
}

User* UserRepository::getUserByEmail(const std::string& email) {
    auto it = std::find_if(users.begin(), users.end(), [&email](const User& user) {
        return user.getEmail() == email;
    });
    
    return (it != users.end()) ? &(*it) : nullptr;
}

const User* UserRepository::getUserByEmail(const std::string& email) const {
    auto it = std::find_if(users.begin(), users.end(), [&email](const User& user) {
        return user.getEmail() == email;
    });
    
    return (it != users.end()) ? &(*it) : nullptr;
}

User UserRepository::addUser(const User& user) {
    User newUser(nextId++, user.getName(), user.getEmail(), "", user.getRole());
    users.push_back(newUser);
    return newUser;
}

bool UserRepository::updateUser(const User& user) {
    auto it = std::find_if(users.begin(), users.end(), [&user](const User& u) {
        return u.getId() == user.getId();
    });
    
    if (it != users.end()) {
        *it = user;
        return true;
    }
    
    return false;
}

bool UserRepository::deleteUser(int id) {
    auto it = std::find_if(users.begin(), users.end(), [id](const User& user) {
        return user.getId() == id;
    });
    
    if (it != users.end()) {
        users.erase(it);
        return true;
    }
    
    return false;
}

User* UserRepository::authenticateUser(const std::string& email, const std::string& password) {
    User* user = getUserByEmail(email);
    if (user && user->checkPassword(password)) {
        return user;
    }
    return nullptr;
}
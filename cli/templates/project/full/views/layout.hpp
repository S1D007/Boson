#pragma once

#include <string>
#include <map>
#include <vector>
#include "../models/user.hpp"

namespace Views {

class Layout {
public:
    static std::string render(const std::string& title, const std::string& content, const std::map<std::string, std::string>& data = {}) {
        return R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>)" + title + R"( - )" + (data.count("project_name") ? data.at("project_name") : "{{.ProjectName}}") + R"(</title>
    <link rel="stylesheet" href="/css/styles.css">
</head>
<body>
    <header>
        <div class="container">
            <h1>)" + (data.count("project_name") ? data.at("project_name") : "{{.ProjectName}}") + R"(</h1>
            <nav>
                <ul>
                    <li><a href="/">Home</a></li>
                    <li><a href="/users">Users</a></li>
                    <li><a href="/api">API</a></li>
                </ul>
            </nav>
        </div>
    </header>
    
    <main>
        <div class="container">
)" + content + R"(
        </div>
    </main>
    
    <footer>
        <div class="container">
            <p>&copy; )" + (data.count("year") ? data.at("year") : "2025") + R"( )" + 
            (data.count("project_name") ? data.at("project_name") : "{{.ProjectName}}") + 
            R"( - Built with Boson Framework</p>
        </div>
    </footer>
    
    <script src="/js/app.js"></script>
</body>
</html>)";
    }
    
    static std::string renderUserList(const std::vector<User>& users) {
        std::string content = R"(
            <h2>Users</h2>
            <div class="actions">
                <a href="/users/new" class="btn btn-primary">Add New User</a>
            </div>
            
            <div id="user-list" class="user-list">)";
            
        if (users.empty()) {
            content += R"(
                <p>No users found.</p>)";
        } else {
            for (const auto& user : users) {
                content += R"(
                <div class="user-card" data-user-id=")" + std::to_string(user.id) + R"(">
                    <h3>)" + user.name + R"(</h3>
                    <p class="email">)" + user.email + R"(</p>
                    <p>Status: <span class="status )" + (user.active ? "active" : "inactive") + R"(">)" + 
                    (user.active ? "Active" : "Inactive") + R"(</span></p>
                    <div class="user-actions">
                        <a href="/users/)" + std::to_string(user.id) + R"(" class="btn btn-primary">View</a>
                        <a href="/users/)" + std::to_string(user.id) + R"(/edit" class="btn btn-secondary">Edit</a>
                        <button class="btn btn-danger delete-user" data-user-id=")" + std::to_string(user.id) + R"(">Delete</button>
                    </div>
                </div>)";
            }
        }
        
        content += R"(
            </div>)";
            
        return render("Users", content);
    }
    
    static std::string renderUserForm(const User& user = {}, bool isEdit = false) {
        std::string formTitle = isEdit ? "Edit User" : "Create New User";
        std::string actionUrl = isEdit ? "/api/users/" + std::to_string(user.id) : "/api/users";
        std::string submitText = isEdit ? "Update User" : "Create User";
        
        std::string content = R"(
            <h2>)" + formTitle + R"(</h2>
            
            <div class="error-message" style="display: none; color: red; margin-bottom: 15px;"></div>
            
            <form id="user-form" data-user-id=")" + (isEdit ? std::to_string(user.id) : "") + R"(">
                <div class="form-group">
                    <label for="name">Name</label>
                    <input type="text" id="name" name="name" value=")" + user.name + R"(" required>
                </div>
                
                <div class="form-group">
                    <label for="email">Email</label>
                    <input type="email" id="email" name="email" value=")" + user.email + R"(" required>
                </div>
                
                <div class="form-group">
                    <label>
                        <input type="checkbox" id="active" name="active" )" + (user.active ? "checked" : "") + R"(>
                        Active
                    </label>
                </div>
                
                <div class="form-actions">
                    <button type="submit" class="btn btn-primary">)" + submitText + R"(</button>
                    <a href="/users" class="btn btn-secondary">Cancel</a>
                </div>
            </form>)";
            
        return render(formTitle, content);
    }
    
    static std::string renderUserDetail(const User& user) {
        std::string content = R"(
            <div class="user-detail">
                <h2>)" + user.name + R"(</h2>
                
                <div class="user-info">
                    <p><strong>ID:</strong> )" + std::to_string(user.id) + R"(</p>
                    <p><strong>Email:</strong> )" + user.email + R"(</p>
                    <p><strong>Status:</strong> <span class="status )" + (user.active ? "active" : "inactive") + R"(">)" + 
                    (user.active ? "Active" : "Inactive") + R"(</span></p>
                </div>
                
                <div class="user-actions">
                    <a href="/users/)" + std::to_string(user.id) + R"(/edit" class="btn btn-primary">Edit</a>
                    <button class="btn btn-danger delete-user" data-user-id=")" + std::to_string(user.id) + R"(">Delete</button>
                    <a href="/users" class="btn btn-secondary">Back to Users</a>
                </div>
            </div>)";
            
        return render(user.name, content);
    }
};

} // namespace Views
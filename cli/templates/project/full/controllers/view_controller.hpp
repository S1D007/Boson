#pragma once

#include <boson/controller.hpp>
#include <boson/request.hpp>
#include <boson/response.hpp>
#include <string>
#include <memory>
#include "../services/user_service.hpp"
#include "../views/layout.hpp"

class ViewController : public boson::Controller {
public:
    ViewController() : userService(std::make_shared<UserService>()) {}
    virtual ~ViewController() = default;

    std::string basePath() const override { 
        return ""; 
    }

    void home(const boson::Request& req, boson::Response& res) {
        std::string content = R"(
            <section class="hero">
                <h2>Welcome to {{.ProjectName}}</h2>
                <p>A modern C++ web application built with Boson Framework</p>
                <div class="buttons">
                    <a href="/api" class="btn btn-primary">API Documentation</a>
                    <a href="/users" class="btn btn-secondary">View Users</a>
                </div>
            </section>
            
            <section class="features">
                <h3>Features</h3>
                <div class="feature-grid">
                    <div class="feature">
                        <h4>Fast C++ Backend</h4>
                        <p>Powered by the Boson Framework for high performance</p>
                    </div>
                    <div class="feature">
                        <h4>RESTful API</h4>
                        <p>Comprehensive API for easy integration</p>
                    </div>
                    <div class="feature">
                        <h4>Modern Frontend</h4>
                        <p>Responsive design with clean UI</p>
                    </div>
                    <div class="feature">
                        <h4>Scalable Architecture</h4>
                        <p>Built for performance and maintainability</p>
                    </div>
                </div>
            </section>
        )";
        
        std::map<std::string, std::string> data = {
            {"project_name", "{{.ProjectName}}"},
            {"year", "2025"}
        };
        
        res.header("Content-Type", "text/html")
           .send(Views::Layout::render("Home", content, data));
    }

    void userList(const boson::Request& req, boson::Response& res) {
        auto users = userService->getAllUsers();
        res.header("Content-Type", "text/html")
           .send(Views::Layout::renderUserList(users));
    }

    void userDetail(const boson::Request& req, boson::Response& res) {
        std::string id = req.param("id");
        
        try {
            int userId = std::stoi(id);
            auto user = userService->getUserById(userId);
            
            if (user.id == 0) {
                res.status(404)
                   .header("Content-Type", "text/html")
                   .send(Views::Layout::render("Not Found", "<h2>User not found</h2><p>The requested user does not exist.</p>"));
                return;
            }
            
            res.header("Content-Type", "text/html")
               .send(Views::Layout::renderUserDetail(user));
        } catch (const std::exception& e) {
            res.status(400)
               .header("Content-Type", "text/html")
               .send(Views::Layout::render("Error", "<h2>Invalid user ID</h2><p>Please provide a valid user ID.</p>"));
        }
    }

    void newUserForm(const boson::Request& req, boson::Response& res) {
        res.header("Content-Type", "text/html")
           .send(Views::Layout::renderUserForm());
    }

    void editUserForm(const boson::Request& req, boson::Response& res) {
        std::string id = req.param("id");
        
        try {
            int userId = std::stoi(id);
            auto user = userService->getUserById(userId);
            
            if (user.id == 0) {
                res.status(404)
                   .header("Content-Type", "text/html")
                   .send(Views::Layout::render("Not Found", "<h2>User not found</h2><p>The requested user does not exist.</p>"));
                return;
            }
            
            res.header("Content-Type", "text/html")
               .send(Views::Layout::renderUserForm(user, true));
        } catch (const std::exception& e) {
            res.status(400)
               .header("Content-Type", "text/html")
               .send(Views::Layout::render("Error", "<h2>Invalid user ID</h2><p>Please provide a valid user ID.</p>"));
        }
    }

private:
    std::shared_ptr<UserService> userService;
};
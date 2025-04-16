#pragma once

#include <boson/controller.hpp>
#include <boson/request.hpp>
#include <boson/response.hpp>
#include <string>

class HelloController : public boson::Controller {
public:
    HelloController() = default;
    virtual ~HelloController() = default;

    std::string basePath() const override { 
        return "/api"; 
    }

    // Default route handler
    void index(const boson::Request& req, boson::Response& res) {
        res.jsonObject({
            {"message", "Welcome to {{.ProjectName}}!"},
            {"version", "1.0.0"},
            {"framework", "Boson C++ Web Framework"}
        });
    }

    // Simple hello handler
    void hello(const boson::Request& req, boson::Response& res) {
        res.send("Hello, World!");
    }

    // Handler with path parameters
    void helloName(const boson::Request& req, boson::Response& res) {
        std::string name = req.param("name");
        res.jsonObject({
            {"message", "Hello, " + name + "!"},
            {"timestamp", std::time(nullptr)}
        });
    }
};
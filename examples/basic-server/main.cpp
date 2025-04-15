#include "boson/boson.hpp"
#include <iostream>
#include <string>
#include <memory>
#include "../../include/external/json.hpp"


class UserController : public boson::Controller {
public:
    UserController() {
        
        registerRoute(boson::HttpMethod::GET, "/", "list_users", 
            [this](const boson::Request& req, boson::Response& res) {
                this->listUsers(req, res);
            });
        
        registerRoute(boson::HttpMethod::GET, "/:id", "get_user_by_id", 
            [this](const boson::Request& req, boson::Response& res) {
                this->getUserById(req, res);
            });
        
        registerRoute(boson::HttpMethod::POST, "/", "create_user", 
            [this](const boson::Request& req, boson::Response& res) {
                this->createUser(req, res);
            });
        
        registerRoute(boson::HttpMethod::PUT, "/:id", "update_user", 
            [this](const boson::Request& req, boson::Response& res) {
                this->updateUser(req, res);
            });
        
        registerRoute(boson::HttpMethod::DELETE, "/:id", "delete_user", 
            [this](const boson::Request& req, boson::Response& res) {
                this->deleteUser(req, res);
            });
    }
    
    virtual ~UserController() = default;
    
    
    std::string basePath() const override {
        return "/users";
    }
    
private:
    
    void listUsers(const boson::Request& req, boson::Response& res) {
        
        nlohmann::json user1 = {
            {"id", 1},
            {"name", "John"},
            {"email", "john@example.com"}
        };
        
        nlohmann::json user2 = {
            {"id", 2},
            {"name", "Jane"},
            {"email", "jane@example.com"}
        };
        
        
        nlohmann::json users = nlohmann::json::array({user1, user2});
        
        
        nlohmann::json response = {
            {"users", users},
            {"total", 2}
        };
        
        res.status(200).jsonObject(response);
    }
    
    
    void getUserById(const boson::Request& req, boson::Response& res) {
        std::string id = req.param("id");
        
        
        nlohmann::json roles = {"user", "admin"};
        nlohmann::json user = {
            {"id", std::stoi(id)},
            {"name", "John Doe"},
            {"email", "john.doe@example.com"},
            {"roles", roles}
        };
        
        
        nlohmann::json response = {
            {"user", user}
        };
        
        res.status(200).jsonObject(response);
    }
    
    
    void createUser(const boson::Request& req, boson::Response& res) {
        
        nlohmann::json requestBody = req.json();
        
        
        std::string name = "New User";
        std::string email = "new@example.com";
        
        if (!requestBody.empty()) {
            if (requestBody.contains("name")) {
                name = requestBody["name"].get<std::string>();
            }
            if (requestBody.contains("email")) {
                email = requestBody["email"].get<std::string>();
            }
        }
        
        
        nlohmann::json user = {
            {"id", 3},  
            {"name", name},
            {"email", email}
        };
        
        
        nlohmann::json response = {
            {"user", user},
            {"message", "User created successfully"}
        };
        
        res.status(201).jsonObject(response);
    }
    
    
    void updateUser(const boson::Request& req, boson::Response& res) {
        std::string id = req.param("id");
        
        nlohmann::json requestBody = req.json();
        
        
        std::string name = "Updated User";
        std::string email = "updated@example.com";
        
        if (!requestBody.empty()) {
            if (requestBody.contains("name")) {
                name = requestBody["name"].get<std::string>();
            }
            if (requestBody.contains("email")) {
                email = requestBody["email"].get<std::string>();
            }
        }
        
        
        nlohmann::json user = {
            {"id", std::stoi(id)},
            {"name", name},
            {"email", email}
        };
        
        
        nlohmann::json response = {
            {"user", user},
            {"message", "User updated successfully"}
        };
        
        res.status(200).jsonObject(response);
    }
    
    
    void deleteUser(const boson::Request& req, boson::Response& res) {
        std::string id = req.param("id");
        
        
        nlohmann::json response = {
            {"message", "User " + id + " deleted successfully"},
            {"deleted", true}
        };
        
        res.status(200).jsonObject(response);
    }
};

int main() {
    
    boson::initialize();
    
    
    boson::Server app;
    
    
    app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
        std::cout << "[" << req.method() << "] " << req.path() << std::endl;
        next();
    });
    
    
    app.get("/", [](const boson::Request& req, boson::Response& res) {
        res.send("Welcome to Boson Framework!");
    });
    
    
    app.get("/json-example", [](const boson::Request& req, boson::Response& res) {
        nlohmann::json nestedData = {
            {"key1", "value1"},
            {"key2", 42},
            {"key3", true}
        };
        
        nlohmann::json exampleJson = {
            {"message", "This is a JSON response"},
            {"success", true},
            {"code", 200},
            {"data", nestedData}
        };
        
        res.jsonObject(exampleJson);
    });
    
    
    app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
        try {
            next();
        } catch (const std::exception& e) {
            nlohmann::json errorJson = {
                {"error", e.what()},
                {"path", req.path()},
                {"status", 500}
            };
            
            res.status(500).jsonObject(errorJson);
        }
    });
    
    
    app.get("/error", [](const boson::Request& req, boson::Response& res) {
        throw boson::HttpError("Something went wrong", 500);
    });
    
    
    boson::Router apiRouter;
    
    apiRouter.get("/status", [](const boson::Request& req, boson::Response& res) {
        nlohmann::json statusJson = {
            {"status", "OK"},
            {"version", "1.0.0"},
            {"timestamp", "2025-04-14T12:00:00Z"},
            {"uptime", 3600}
        };
        
        res.jsonObject(statusJson);
    });
    
    
    apiRouter.get("/echo/:message", [](const boson::Request& req, boson::Response& res) {
        std::string message = req.param("message");
        
        
        nlohmann::json paramsJson = nlohmann::json::object();
        
        
        auto queryParams = req.queryParams();
        for (const auto& param : queryParams) {
            paramsJson[param.first] = param.second;
        }
        
        nlohmann::json response = {
            {"echo", message},
            {"params", paramsJson}
        };
        
        res.jsonObject(response);
    });
    
    
    app.use("/api", apiRouter);
    
    
    boson::Router userRouter;
    
    
    auto userController = std::make_shared<UserController>();
    userController->registerRoutes(userRouter);
    
    
    app.use("", userRouter);
    
    
    std::cout << "Starting server on port 3000..." << std::endl;
    app.configure(3000, "127.0.0.1");
    
    return app.listen();
}
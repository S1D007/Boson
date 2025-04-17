#include <boson/boson.hpp>
#include <iostream>
#include <string>
#include <filesystem>
#include <external/json.hpp>

#include "controllers/api_controller.hpp"
#include "controllers/view_controller.hpp"
#include "middleware/auth_middleware.hpp"
#include "middleware/logger_middleware.hpp"
#include "services/user_service.hpp"

int main() {
    try {
        boson::initialize();
        boson::Server app;
        
        std::filesystem::path publicPath = std::filesystem::current_path() / "public";
        if (std::filesystem::exists(publicPath)) {
            std::unordered_map<std::string, std::string> options = {
                {"cacheControl", "max-age=86400"}
            };
            
            app.use(boson::StaticFiles::create(publicPath.string(), "/", options));
            std::cout << "Serving static files from: " << publicPath << std::endl;
        }
        
        auto loggerMiddleware = std::make_shared<LoggerMiddleware>();
        auto authMiddleware = std::make_shared<AuthMiddleware>();
        
        app.use([loggerMiddleware](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
            std::function<void()> nextFn = [&next]() { next(); };
            loggerMiddleware->process(req, res, nextFn);
        });
        
        auto apiController = std::make_shared<ApiController>();
        auto apiRouter = boson::createRouter(apiController);
        
        apiRouter.get("/users", &ApiController::getUsers);
        apiRouter.get("/users/:id", &ApiController::getUserById);
        apiRouter.post("/users", &ApiController::createUser);
        apiRouter.put("/users/:id", &ApiController::updateUser);
        apiRouter.del("/users/:id", &ApiController::deleteUser);
        
        apiRouter.get("/protected", 
            [authMiddleware](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
                std::function<void()> nextFn = [&next]() { next(); };
                authMiddleware->process(req, res, nextFn);
            },
            [](const boson::Request& req, boson::Response& res) {
                res.jsonObject({
                    {"message", "Protected resource accessed successfully"}
                });
            }
        );
        
        auto viewController = std::make_shared<ViewController>();
        auto viewRouter = boson::createRouter(viewController);
        
        viewRouter.get("/", &ViewController::home);
        viewRouter.get("/users", &ViewController::userList);
        viewRouter.get("/users/new", &ViewController::newUserForm);
        viewRouter.get("/users/:id", &ViewController::userDetail);
        viewRouter.get("/users/:id/edit", &ViewController::editUserForm);
        
        apiRouter.mountOn(&app);
        viewRouter.mountOn(&app);
        
        unsigned int port = 3000;
        std::string host = "127.0.0.1";
        app.configure(port, host);
        
        std::cout << "App server running at http://" << host << ":" << port << std::endl;
        
        return app.listen();
    }
    catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
}
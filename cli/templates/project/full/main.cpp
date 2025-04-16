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
        // Initialize Boson framework
        boson::initialize();
        
        // Create server instance
        boson::Server app;
        
        // Setup static file directory for public assets
        std::filesystem::path publicPath = std::filesystem::current_path() / "public";
        if (std::filesystem::exists(publicPath)) {
            // Using standard route to serve static files
            app.get("/css/*", [publicPath](const boson::Request& req, boson::Response& res) {
                std::string path = req.path().substr(1); // Remove leading slash
                std::filesystem::path filePath = publicPath / path;
                // Add file serving logic here
                res.header("Content-Type", "text/css").send("/* CSS file content */");
            });
            
            app.get("/js/*", [publicPath](const boson::Request& req, boson::Response& res) {
                std::string path = req.path().substr(1); // Remove leading slash
                std::filesystem::path filePath = publicPath / path;
                // Add file serving logic here
                res.header("Content-Type", "application/javascript").send("// JS file content");
            });
            
            std::cout << "Serving static files from: " << publicPath << std::endl;
        }
        
        // Create middleware instances
        auto loggerMiddleware = std::make_shared<LoggerMiddleware>();
        auto authMiddleware = std::make_shared<AuthMiddleware>();
        
        // Add global middleware using function approach
        app.use([loggerMiddleware](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
            // Forward the call to the middleware object
            std::function<void()> nextFn = [&next]() { next(); };
            loggerMiddleware->process(req, res, nextFn);
        });
        
        // API routes setup
        auto apiController = std::make_shared<ApiController>();
        auto apiRouter = boson::createRouter(apiController);
        
        // Define API routes
        apiRouter.get("/users", &ApiController::getUsers);
        apiRouter.get("/users/:id", &ApiController::getUserById);
        apiRouter.post("/users", &ApiController::createUser);
        apiRouter.put("/users/:id", &ApiController::updateUser);
        apiRouter.del("/users/:id", &ApiController::deleteUser);
        
        // Protect API routes with auth middleware
        // Replace the .use() call with route-specific middleware
        apiRouter.get("/protected", 
            [authMiddleware](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
                std::function<void()> nextFn = [&next]() { next(); };
                authMiddleware->process(req, res, nextFn);
            },
            [](const boson::Request& req, boson::Response& res) {
                // Using direct JSON initialization
                res.jsonObject({
                    {"message", "Protected resource accessed successfully"}
                });
            }
        );
        
        // View routes setup
        auto viewController = std::make_shared<ViewController>();
        auto viewRouter = boson::createRouter(viewController);
        
        // Define view routes
        viewRouter.get("/", &ViewController::home);
        viewRouter.get("/users", &ViewController::userList);
        viewRouter.get("/users/new", &ViewController::newUserForm);
        viewRouter.get("/users/:id", &ViewController::userDetail);
        viewRouter.get("/users/:id/edit", &ViewController::editUserForm);
        
        // Mount routers to the server
        apiRouter.mountOn(&app);
        viewRouter.mountOn(&app);
        
        // Configure and start the server
        unsigned int port = 3000;
        std::string host = "127.0.0.1";
        app.configure(port, host);
        
        std::cout << "{{.ProjectName}} server running at http://" << host << ":" << port << std::endl;
        
        return app.listen();
    }
    catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
}
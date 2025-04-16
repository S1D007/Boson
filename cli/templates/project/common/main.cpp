#include <boson/boson.hpp>
#include <iostream>
#include <string>

#include "controllers/hello_controller.hpp"

int main() {
    try {
        // Initialize Boson framework
        boson::initialize();
        
        // Create server instance
        boson::Server app;
        
        // Add basic logging middleware
        app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
            std::cout << "[" << req.method() << "] " << req.path() << std::endl;
            next();
        });
        
        // Register routes from the HelloController
        auto helloController = std::make_shared<HelloController>();
        auto helloRouter = boson::createRouter(helloController);
        helloRouter.get("/", &HelloController::index);
        helloRouter.get("/hello", &HelloController::hello);
        helloRouter.get("/hello/:name", &HelloController::helloName);
        
        // Mount controller to the server
        helloRouter.mountOn(&app);
        
        // Configure and start the server
        app.configure(3000, "127.0.0.1");
        std::cout << "{{.ProjectName}} server running at http://127.0.0.1:3000" << std::endl;
        
        return app.listen();
    }
    catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
}
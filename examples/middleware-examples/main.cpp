#include "boson/boson.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <chrono>
#include <thread>
#include <chrono>

int main() {
    boson::initialize();
    
    boson::Server app;
    
    app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
        std::cout << "[Global Middleware] Processing request: " << req.method() << " " << req.path() << std::endl;
        
        const_cast<boson::Request&>(req).set("requestStartTime", 
            std::chrono::system_clock::now().time_since_epoch().count());
        
        next();
        
        std::cout << "[Global Middleware] Completed request: " << req.path() 
                  << " with status: " << res.getStatusCode() << std::endl;
    });
    
    app.use("/api", [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
        std::cout << "[API Middleware] Request to API endpoint: " << req.path() << std::endl;
        
        res.header("X-API-Version", "1.0");
        
        next();
    });
    
    app.use("/admin", [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
        std::cout << "[Auth Middleware] Checking authorization: " << req.path() << " - " << req.method() << std::endl;
        
        std::string authHeader = req.header("Authorization");
        if (authHeader.empty() || authHeader != "Bearer admin-token") {
            res.status(401).jsonObject({
                {"error", "Unauthorized"},
                {"message", "Admin access requires valid authentication"}
            });
            return;
        }
        
        next();
    });
    
    app.use("/api/data", [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
        std::cout << "[Timing Middleware] Starting timer for: " << req.path() << std::endl;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        next();
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        
        res.header("X-Processing-Time", std::to_string(duration) + "ms");
        
        std::cout << "[Timing Middleware] Request to " << req.path() 
                  << " took " << duration << "ms to process" << std::endl;
    });
    
    app.get("/", [](const boson::Request& req, boson::Response& res) {
        res.send("Welcome to Boson Framework with Enhanced Middleware!");
    });
    
    app.post("/", [](const boson::Request& req, boson::Response& res) {
        res.jsonObject({
            {"message", "POST request received"},
            {"method", req.method()},
            {"path", req.path()}
        });
    });
    
    app.use("/", [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
        std::cout << "[Root Path Middleware] Received " << req.method() << " request to /" << std::endl;
        
        if (req.method() != "GET" && req.method() != "POST" && !res.sent()) {
            res.status(200).jsonObject({
                {"message", "Request handled by catch-all middleware"},
                {"method", req.method()},
                {"path", req.path()}
            });
            return;
        }
        
        next();
    });
    
    app.get("/api/hello", [](const boson::Request& req, boson::Response& res) {
        res.jsonObject({
            {"message", "Hello from the API!"},
            {"path", req.path()},
            {"query", req.queryParams()}
        });
    });
    
    app.get("/api/data", [](const boson::Request& req, boson::Response& res) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::string requestTime = "Unknown";
        if (req.has("requestStartTime")) {
            auto startTime = std::any_cast<long long>(req.get("requestStartTime"));
            requestTime = std::to_string(startTime);
        }
        
        res.jsonObject({
            {"message", "Data endpoint with timing middleware"},
            {"data", {{"value1", 42}, {"value2", "test"}}},
            {"requestStartTime", requestTime}
        });
    });
    
    app.get("/admin/dashboard", [](const boson::Request& req, boson::Response& res) {
        res.jsonObject({
            {"message", "Admin Dashboard"},
            {"status", "You have access to protected content"}
        });
    });
    
    app.post("/admin/dashboard", [](const boson::Request& req, boson::Response& res) {
        res.jsonObject({
            {"message", "Admin Dashboard POST handler"},
            {"status", "POST request processed successfully"}
        });
    });
    
    app.use("/admin", [](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
        if (!res.sent()) {
            res.jsonObject({
                {"message", "Admin request handled by catch-all handler"},
                {"method", req.method()},
                {"path", req.path()}
            });
        } else {
            next();
        }
    });
    
    app.setErrorHandler([](const std::exception& e, const boson::Request& req, boson::Response& res) {
        std::cerr << "Error: " << e.what() << std::endl;
        
        const boson::HttpError* httpError = dynamic_cast<const boson::HttpError*>(&e);
        int statusCode = httpError ? httpError->statusCode() : 500;
        
        res.status(statusCode).jsonObject({
            {"error", true},
            {"message", e.what()},
            {"status", statusCode}
        });
    });
    
    std::cout << "Starting server on http://127.0.0.1:3000" << std::endl;
    std::cout << "Available routes:" << std::endl;
    std::cout << "  - GET /" << std::endl;
    std::cout << "  - GET /api/hello" << std::endl;
    std::cout << "  - GET /api/data" << std::endl;
    std::cout << "  - GET /admin/dashboard (requires 'Authorization: Bearer admin-token' header)" << std::endl;
    
    app.configure(3000, "127.0.0.1");
    return app.listen();
}
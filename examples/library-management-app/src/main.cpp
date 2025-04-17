#include <boson/boson.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <filesystem>
#include "../controllers/book_controller.hpp"
#include "../controllers/user_controller.hpp"
#include "../models/book.hpp"
#include "../models/user.hpp"
#include "../models/loan.hpp"

void setupCorsMiddleware(boson::Server& app) {
    app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
        res.header("Access-Control-Allow-Origin", "*");
        res.header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        
        if (req.method() == "OPTIONS") {
            res.status(204).send("");
            return;
        }
        
        next();
    });
}

void setupLoggingMiddleware(boson::Server& app) {
    app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
        std::cout << "[" << req.method() << "] " << req.path() << std::endl;
        next();
    });
}

void setupErrorHandlingMiddleware(boson::Server& app) {
    app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
        try {
            next();
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            
            res.status(500).jsonObject({
                {"error", "Internal Server Error"},
                {"message", e.what()},
                {"path", req.path()}
            });
        }
    });
}

int main() {
    try {
        boson::initialize();
        
        boson::Server app;
        
        setupCorsMiddleware(app);
        setupLoggingMiddleware(app);
        setupErrorHandlingMiddleware(app);
        
        // API status endpoint
        app.get("/api/status", [](const boson::Request& req, boson::Response& res) {
            res.jsonObject({
                {"status", "ok"},
                {"version", "1.0.0"},
                {"message", "Library Management API is running"}
            });
        });
        
        // Setup book controller routes
        auto bookController = std::make_shared<BookController>();
        auto bookRouter = boson::createRouter(bookController);
        
        bookRouter
            .get("/", &BookController::getAllBooks)
            .get("/:id", &BookController::getBookById)
            .get("/search", &BookController::searchBooks)
            .post("/", &BookController::createBook)
            .put("/:id", &BookController::updateBook)
            .del("/:id", &BookController::deleteBook)
            .post("/checkout", &BookController::checkoutBook)
            .post("/return", &BookController::returnBook);
        
        // Setup user controller routes
        auto userController = std::make_shared<UserController>();
        auto userRouter = boson::createRouter(userController);
        
        userRouter
            .get("/", &UserController::getAllUsers)
            .get("/:id", &UserController::getUserById)
            .post("/", &UserController::createUser)
            .put("/:id", &UserController::updateUser)
            .del("/:id", &UserController::deleteUser)
            .post("/login", &UserController::login)
            .get("/:id/loans", &UserController::getUserLoans);
        
        // Mount controllers to the server
        bookRouter.mountOn(&app);
        userRouter.mountOn(&app);
        
        
        const int PORT = 8080;
        const std::string HOST = "0.0.0.0";
        
        std::cout << "Starting Library Management Server on http://" << HOST << ":" << PORT << std::endl;
        app.configure(PORT, HOST);
        
        return app.listen();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
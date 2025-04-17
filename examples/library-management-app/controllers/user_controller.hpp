#pragma once

#include <boson/controller.hpp>
#include <boson/request.hpp>
#include <boson/response.hpp>
#include "../models/user.hpp"
#include "../models/loan.hpp"
#include <string>

class UserController : public boson::Controller {
public:
    UserController() = default;
    virtual ~UserController() = default;
    
    std::string basePath() const override;
    
    void getAllUsers(const boson::Request& req, boson::Response& res);
    void getUserById(const boson::Request& req, boson::Response& res);
    void createUser(const boson::Request& req, boson::Response& res);
    void updateUser(const boson::Request& req, boson::Response& res);
    void deleteUser(const boson::Request& req, boson::Response& res);
    void login(const boson::Request& req, boson::Response& res);
    void getUserLoans(const boson::Request& req, boson::Response& res);
};
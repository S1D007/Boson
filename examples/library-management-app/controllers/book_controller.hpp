#pragma once

#include <boson/controller.hpp>
#include <boson/request.hpp>
#include <boson/response.hpp>
#include "../models/book.hpp"
#include <string>

class BookController : public boson::Controller {
public:
    BookController() = default;
    virtual ~BookController() = default;
    
    std::string basePath() const override;
    
    void getAllBooks(const boson::Request& req, boson::Response& res);
    void getBookById(const boson::Request& req, boson::Response& res);
    void searchBooks(const boson::Request& req, boson::Response& res);
    void createBook(const boson::Request& req, boson::Response& res);
    void updateBook(const boson::Request& req, boson::Response& res);
    void deleteBook(const boson::Request& req, boson::Response& res);
    void checkoutBook(const boson::Request& req, boson::Response& res);
    void returnBook(const boson::Request& req, boson::Response& res);
};
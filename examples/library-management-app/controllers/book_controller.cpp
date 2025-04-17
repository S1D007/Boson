#include "book_controller.hpp"
#include "../models/book.hpp"
#include "../models/loan.hpp"
#include <stdexcept>
#include <cctype>
#include <string>
#include <vector>
#include <algorithm>

std::string BookController::basePath() const {
    return "/books";
}

void BookController::getAllBooks(const boson::Request& req, boson::Response& res) {
    auto& bookRepo = BookRepository::getInstance();
    auto books = bookRepo.getAllBooks();
    
    nlohmann::json booksJson = nlohmann::json::array();
    
    for (const auto& book : books) {
        booksJson.push_back(book.toJson());
    }
    
    res.status(200).jsonObject({
        {"books", booksJson},
        {"total", books.size()}
    });
}

void BookController::getBookById(const boson::Request& req, boson::Response& res) {
    std::string idStr = req.param("id");
    int id;
    
    try {
        id = std::stoi(idStr);
    } catch (const std::exception&) {
        res.status(400).jsonObject({
            {"error", "Invalid book ID format"}
        });
        return;
    }
    
    auto& bookRepo = BookRepository::getInstance();
    const Book* book = bookRepo.getBookById(id);
    
    if (!book) {
        res.status(404).jsonObject({
            {"error", "Book not found"},
            {"id", id}
        });
        return;
    }
    
    res.status(200).jsonObject(book->toJson());
}

void BookController::searchBooks(const boson::Request& req, boson::Response& res) {
    auto& bookRepo = BookRepository::getInstance();
    std::vector<Book> results;
    
    auto queryParams = req.queryParams();
    
    if (queryParams.find("title") != queryParams.end()) {
        results = bookRepo.findBooksByTitle(queryParams["title"]);
    } else if (queryParams.find("author") != queryParams.end()) {
        results = bookRepo.findBooksByAuthor(queryParams["author"]);
    } else if (queryParams.find("genre") != queryParams.end()) {
        results = bookRepo.findBooksByGenre(queryParams["genre"]);
    } else {
        res.status(400).jsonObject({
            {"error", "Search requires one of the following parameters: title, author, or genre"}
        });
        return;
    }
    
    nlohmann::json booksJson = nlohmann::json::array();
    
    for (const auto& book : results) {
        booksJson.push_back(book.toJson());
    }
    
    res.status(200).jsonObject({
        {"books", booksJson},
        {"total", results.size()}
    });
}

void BookController::createBook(const boson::Request& req, boson::Response& res) {
    try {
        auto json = req.json();
        
        if (!json.contains("title") || !json.contains("author") || 
            !json.contains("isbn") || !json.contains("genre") || 
            !json.contains("publicationYear")) {
            res.status(400).jsonObject({
                {"error", "Missing required fields"},
                {"required", {"title", "author", "isbn", "genre", "publicationYear"}}
            });
            return;
        }
        
        Book newBook(
            0, // Temporary ID, will be replaced
            json["title"],
            json["author"],
            json["isbn"],
            json["genre"],
            json["publicationYear"]
        );
        
        auto& bookRepo = BookRepository::getInstance();
        Book createdBook = bookRepo.addBook(newBook);
        
        res.status(201).jsonObject({
            {"message", "Book created successfully"},
            {"book", createdBook.toJson()}
        });
    } catch (const std::exception& e) {
        res.status(400).jsonObject({
            {"error", "Failed to create book"},
            {"message", e.what()}
        });
    }
}

void BookController::updateBook(const boson::Request& req, boson::Response& res) {
    std::string idStr = req.param("id");
    int id;
    
    try {
        id = std::stoi(idStr);
    } catch (const std::exception&) {
        res.status(400).jsonObject({
            {"error", "Invalid book ID format"}
        });
        return;
    }
    
    auto& bookRepo = BookRepository::getInstance();
    Book* book = bookRepo.getBookById(id);
    
    if (!book) {
        res.status(404).jsonObject({
            {"error", "Book not found"},
            {"id", id}
        });
        return;
    }
    
    try {
        auto json = req.json();
        
        Book updatedBook(
            id,
            json.contains("title") ? json["title"].get<std::string>() : book->getTitle(),
            json.contains("author") ? json["author"].get<std::string>() : book->getAuthor(),
            json.contains("isbn") ? json["isbn"].get<std::string>() : book->getIsbn(),
            json.contains("genre") ? json["genre"].get<std::string>() : book->getGenre(),
            json.contains("publicationYear") ? json["publicationYear"].get<int>() : book->getPublicationYear(),
            book->isAvailable()
        );
        
        if (bookRepo.updateBook(updatedBook)) {
            res.status(200).jsonObject({
                {"message", "Book updated successfully"},
                {"book", updatedBook.toJson()}
            });
        } else {
            res.status(500).jsonObject({
                {"error", "Failed to update book"}
            });
        }
    } catch (const std::exception& e) {
        res.status(400).jsonObject({
            {"error", "Invalid request data"},
            {"message", e.what()}
        });
    }
}

void BookController::deleteBook(const boson::Request& req, boson::Response& res) {
    std::string idStr = req.param("id");
    int id;
    
    try {
        id = std::stoi(idStr);
    } catch (const std::exception&) {
        res.status(400).jsonObject({
            {"error", "Invalid book ID format"}
        });
        return;
    }
    
    auto& bookRepo = BookRepository::getInstance();
    const Book* book = bookRepo.getBookById(id);
    
    if (!book) {
        res.status(404).jsonObject({
            {"error", "Book not found"},
            {"id", id}
        });
        return;
    }
    
    if (bookRepo.deleteBook(id)) {
        res.status(200).jsonObject({
            {"message", "Book deleted successfully"},
            {"id", id}
        });
    } else {
        res.status(500).jsonObject({
            {"error", "Failed to delete book"}
        });
    }
}

void BookController::checkoutBook(const boson::Request& req, boson::Response& res) {
    try {
        auto json = req.json();
        
        if (!json.contains("bookId") || !json.contains("userId")) {
            res.status(400).jsonObject({
                {"error", "Missing required fields"},
                {"required", {"bookId", "userId"}}
            });
            return;
        }
        
        int bookId = json["bookId"];
        int userId = json["userId"];
        int daysToKeep = json.contains("daysToKeep") ? json["daysToKeep"].get<int>() : 14;
        
        auto& loanRepo = LoanRepository::getInstance();
        auto loan = loanRepo.checkoutBook(userId, bookId, daysToKeep);
        
        res.status(200).jsonObject({
            {"message", "Book checked out successfully"},
            {"loan", loan.toJson()}
        });
    } catch (const std::exception& e) {
        res.status(400).jsonObject({
            {"error", "Failed to checkout book"},
            {"message", e.what()}
        });
    }
}

void BookController::returnBook(const boson::Request& req, boson::Response& res) {
    try {
        auto json = req.json();
        
        if (!json.contains("loanId")) {
            res.status(400).jsonObject({
                {"error", "Missing required field: loanId"}
            });
            return;
        }
        
        int loanId = json["loanId"];
        
        auto& loanRepo = LoanRepository::getInstance();
        bool success = loanRepo.returnBook(loanId);
        
        if (success) {
            res.status(200).jsonObject({
                {"message", "Book returned successfully"},
                {"loanId", loanId}
            });
        } else {
            res.status(400).jsonObject({
                {"error", "Failed to return book"},
                {"loanId", loanId}
            });
        }
    } catch (const std::exception& e) {
        res.status(400).jsonObject({
            {"error", "Invalid request data"},
            {"message", e.what()}
        });
    }
}
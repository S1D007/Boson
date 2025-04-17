#pragma once

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include "../include/external/json.hpp"

class Book {
public:
    Book() = default;
    Book(int id, std::string title, std::string author, std::string isbn, std::string genre, int publicationYear, bool available = true);
    
    int getId() const;
    const std::string& getTitle() const;
    const std::string& getAuthor() const;
    const std::string& getIsbn() const;
    const std::string& getGenre() const;
    int getPublicationYear() const;
    bool isAvailable() const;
    
    void setAvailable(bool available);
    
    nlohmann::json toJson() const;
    static Book fromJson(const nlohmann::json& json);
    
private:
    int id;
    std::string title;
    std::string author;
    std::string isbn;
    std::string genre;
    int publicationYear;
    bool available;
};

class BookRepository {
public:
    static BookRepository& getInstance();
    
    std::vector<Book> getAllBooks() const;
    Book* getBookById(int id);
    const Book* getBookById(int id) const;
    std::vector<Book> findBooksByTitle(const std::string& title) const;
    std::vector<Book> findBooksByAuthor(const std::string& author) const;
    std::vector<Book> findBooksByGenre(const std::string& genre) const;
    
    Book addBook(const Book& book);
    bool updateBook(const Book& book);
    bool deleteBook(int id);
    bool checkoutBook(int id);
    bool returnBook(int id);
    
private:
    BookRepository();
    std::vector<Book> books;
    int nextId = 1;
};
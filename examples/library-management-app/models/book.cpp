#include "book.hpp"
#include <algorithm>
#include <cctype>
#include <stdexcept>

Book::Book(int id, std::string title, std::string author, std::string isbn, std::string genre, int publicationYear, bool available)
    : id(id), title(std::move(title)), author(std::move(author)), isbn(std::move(isbn)), genre(std::move(genre)), publicationYear(publicationYear), available(available) {}

int Book::getId() const { return id; }
const std::string& Book::getTitle() const { return title; }
const std::string& Book::getAuthor() const { return author; }
const std::string& Book::getIsbn() const { return isbn; }
const std::string& Book::getGenre() const { return genre; }
int Book::getPublicationYear() const { return publicationYear; }
bool Book::isAvailable() const { return available; }
void Book::setAvailable(bool value) { available = value; }

nlohmann::json Book::toJson() const {
    return {
        {"id", id},
        {"title", title},
        {"author", author},
        {"isbn", isbn},
        {"genre", genre},
        {"publicationYear", publicationYear},
        {"available", available}
    };
}

Book Book::fromJson(const nlohmann::json& json) {
    return Book(
        json["id"],
        json["title"],
        json["author"],
        json["isbn"],
        json["genre"],
        json["publicationYear"],
        json.contains("available") ? json["available"].get<bool>() : true
    );
}

BookRepository::BookRepository() {
    books = {
        {1, "To Kill a Mockingbird", "Harper Lee", "9780061120084", "Classic", 1960, true},
        {2, "1984", "George Orwell", "9780451524935", "Dystopian", 1949, true},
        {3, "Pride and Prejudice", "Jane Austen", "9780141439518", "Romance", 1813, true},
        {4, "The Great Gatsby", "F. Scott Fitzgerald", "9780743273565", "Classic", 1925, true},
        {5, "The Hobbit", "J.R.R. Tolkien", "9780547928227", "Fantasy", 1937, true},
        {6, "Harry Potter and the Philosopher's Stone", "J.K. Rowling", "9780747532743", "Fantasy", 1997, true},
        {7, "The Catcher in the Rye", "J.D. Salinger", "9780316769488", "Coming-of-age", 1951, true},
        {8, "Lord of the Flies", "William Golding", "9780399501487", "Adventure", 1954, true},
        {9, "Animal Farm", "George Orwell", "9780451526342", "Political Satire", 1945, true},
        {10, "Brave New World", "Aldous Huxley", "9780060850524", "Dystopian", 1932, true}
    };
    nextId = books.size() + 1;
}

BookRepository& BookRepository::getInstance() {
    static BookRepository instance;
    return instance;
}

std::vector<Book> BookRepository::getAllBooks() const {
    return books;
}

Book* BookRepository::getBookById(int id) {
    auto it = std::find_if(books.begin(), books.end(), [id](const Book& book) {
        return book.getId() == id;
    });
    
    return (it != books.end()) ? &(*it) : nullptr;
}

const Book* BookRepository::getBookById(int id) const {
    auto it = std::find_if(books.begin(), books.end(), [id](const Book& book) {
        return book.getId() == id;
    });
    
    return (it != books.end()) ? &(*it) : nullptr;
}

std::vector<Book> BookRepository::findBooksByTitle(const std::string& title) const {
    std::vector<Book> result;
    std::string lowerTitle = title;
    std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
    
    for (const auto& book : books) {
        std::string bookTitle = book.getTitle();
        std::transform(bookTitle.begin(), bookTitle.end(), bookTitle.begin(), ::tolower);
        
        if (bookTitle.find(lowerTitle) != std::string::npos) {
            result.push_back(book);
        }
    }
    
    return result;
}

std::vector<Book> BookRepository::findBooksByAuthor(const std::string& author) const {
    std::vector<Book> result;
    std::string lowerAuthor = author;
    std::transform(lowerAuthor.begin(), lowerAuthor.end(), lowerAuthor.begin(), ::tolower);
    
    for (const auto& book : books) {
        std::string bookAuthor = book.getAuthor();
        std::transform(bookAuthor.begin(), bookAuthor.end(), bookAuthor.begin(), ::tolower);
        
        if (bookAuthor.find(lowerAuthor) != std::string::npos) {
            result.push_back(book);
        }
    }
    
    return result;
}

std::vector<Book> BookRepository::findBooksByGenre(const std::string& genre) const {
    std::vector<Book> result;
    std::string lowerGenre = genre;
    std::transform(lowerGenre.begin(), lowerGenre.end(), lowerGenre.begin(), ::tolower);
    
    for (const auto& book : books) {
        std::string bookGenre = book.getGenre();
        std::transform(bookGenre.begin(), bookGenre.end(), bookGenre.begin(), ::tolower);
        
        if (bookGenre.find(lowerGenre) != std::string::npos) {
            result.push_back(book);
        }
    }
    
    return result;
}

Book BookRepository::addBook(const Book& book) {
    Book newBook(nextId++, book.getTitle(), book.getAuthor(), book.getIsbn(), book.getGenre(), book.getPublicationYear());
    books.push_back(newBook);
    return newBook;
}

bool BookRepository::updateBook(const Book& book) {
    auto it = std::find_if(books.begin(), books.end(), [&book](const Book& b) {
        return b.getId() == book.getId();
    });
    
    if (it != books.end()) {
        *it = book;
        return true;
    }
    
    return false;
}

bool BookRepository::deleteBook(int id) {
    auto it = std::find_if(books.begin(), books.end(), [id](const Book& book) {
        return book.getId() == id;
    });
    
    if (it != books.end()) {
        books.erase(it);
        return true;
    }
    
    return false;
}

bool BookRepository::checkoutBook(int id) {
    Book* book = getBookById(id);
    if (book && book->isAvailable()) {
        book->setAvailable(false);
        return true;
    }
    return false;
}

bool BookRepository::returnBook(int id) {
    Book* book = getBookById(id);
    if (book && !book->isAvailable()) {
        book->setAvailable(true);
        return true;
    }
    return false;
}
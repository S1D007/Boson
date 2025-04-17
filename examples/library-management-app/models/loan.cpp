#include "loan.hpp"
#include "book.hpp"
#include <algorithm>
#include <stdexcept>

Loan::Loan(int id, int userId, int bookId, time_t dueDate, time_t checkoutDate, Status status, time_t returnDate)
    : id(id), userId(userId), bookId(bookId), dueDate(dueDate), checkoutDate(checkoutDate), returnDate(returnDate), status(status) {}

int Loan::getId() const { return id; }
int Loan::getUserId() const { return userId; }
int Loan::getBookId() const { return bookId; }
time_t Loan::getDueDate() const { return dueDate; }
time_t Loan::getCheckoutDate() const { return checkoutDate; }
time_t Loan::getReturnDate() const { return returnDate; }
Loan::Status Loan::getStatus() const { return status; }

void Loan::setReturnDate(time_t date) {
    returnDate = date;
}

void Loan::setStatus(Status newStatus) {
    status = newStatus;
}

nlohmann::json Loan::toJson() const {
    std::string statusStr;
    switch (status) {
        case Status::ACTIVE: statusStr = "ACTIVE"; break;
        case Status::RETURNED: statusStr = "RETURNED"; break;
        case Status::OVERDUE: statusStr = "OVERDUE"; break;
    }
    
    return {
        {"id", id},
        {"userId", userId},
        {"bookId", bookId},
        {"dueDate", dueDate},
        {"checkoutDate", checkoutDate},
        {"returnDate", returnDate},
        {"status", statusStr}
    };
}

Loan Loan::fromJson(const nlohmann::json& json) {
    Status status = Status::ACTIVE;
    if (json.contains("status")) {
        std::string statusStr = json["status"];
        if (statusStr == "RETURNED") status = Status::RETURNED;
        else if (statusStr == "OVERDUE") status = Status::OVERDUE;
    }
    
    return Loan(
        json["id"],
        json["userId"],
        json["bookId"],
        json["dueDate"],
        json.contains("checkoutDate") ? json["checkoutDate"].get<time_t>() : time(nullptr),
        status,
        json.contains("returnDate") ? json["returnDate"].get<time_t>() : 0
    );
}

LoanRepository::LoanRepository() {
    time_t now = time(nullptr);
    time_t oneDay = 24 * 60 * 60;
    
    loans = {
        {1, 3, 1, now + oneDay * 7, now - oneDay * 7},
        {2, 4, 2, now - oneDay * 3, now - oneDay * 10, Loan::Status::OVERDUE}
    };
    
    nextId = loans.size() + 1;
}

LoanRepository& LoanRepository::getInstance() {
    static LoanRepository instance;
    return instance;
}

std::vector<Loan> LoanRepository::getAllLoans() const {
    return loans;
}

std::vector<Loan> LoanRepository::getLoansByUserId(int userId) const {
    std::vector<Loan> result;
    
    for (const auto& loan : loans) {
        if (loan.getUserId() == userId) {
            result.push_back(loan);
        }
    }
    
    return result;
}

std::vector<Loan> LoanRepository::getActiveLoans() const {
    std::vector<Loan> result;
    
    for (const auto& loan : loans) {
        if (loan.getStatus() == Loan::Status::ACTIVE || loan.getStatus() == Loan::Status::OVERDUE) {
            result.push_back(loan);
        }
    }
    
    return result;
}

std::vector<Loan> LoanRepository::getOverdueLoans() const {
    std::vector<Loan> result;
    
    for (const auto& loan : loans) {
        if (loan.getStatus() == Loan::Status::OVERDUE) {
            result.push_back(loan);
        }
    }
    
    return result;
}

Loan* LoanRepository::getLoanById(int id) {
    auto it = std::find_if(loans.begin(), loans.end(), [id](const Loan& loan) {
        return loan.getId() == id;
    });
    
    return (it != loans.end()) ? &(*it) : nullptr;
}

const Loan* LoanRepository::getLoanById(int id) const {
    auto it = std::find_if(loans.begin(), loans.end(), [id](const Loan& loan) {
        return loan.getId() == id;
    });
    
    return (it != loans.end()) ? &(*it) : nullptr;
}

Loan* LoanRepository::getActiveLoanForBook(int bookId) {
    auto it = std::find_if(loans.begin(), loans.end(), [bookId](const Loan& loan) {
        return loan.getBookId() == bookId && 
              (loan.getStatus() == Loan::Status::ACTIVE || loan.getStatus() == Loan::Status::OVERDUE);
    });
    
    return (it != loans.end()) ? &(*it) : nullptr;
}

Loan LoanRepository::checkoutBook(int userId, int bookId, int daysToKeep) {
    auto& bookRepo = BookRepository::getInstance();
    Book* book = bookRepo.getBookById(bookId);
    
    if (!book) {
        throw std::runtime_error("Book not found");
    }
    
    if (!book->isAvailable()) {
        throw std::runtime_error("Book is not available");
    }
    
    time_t now = time(nullptr);
    time_t dueDate = now + (daysToKeep * 24 * 60 * 60);
    
    Loan newLoan(nextId++, userId, bookId, dueDate, now);
    loans.push_back(newLoan);
    
    book->setAvailable(false);
    
    return newLoan;
}

bool LoanRepository::returnBook(int loanId) {
    Loan* loan = getLoanById(loanId);
    if (!loan || loan->getStatus() == Loan::Status::RETURNED) {
        return false;
    }
    
    auto& bookRepo = BookRepository::getInstance();
    Book* book = bookRepo.getBookById(loan->getBookId());
    
    if (!book) {
        return false;
    }
    
    loan->setStatus(Loan::Status::RETURNED);
    loan->setReturnDate(time(nullptr));
    book->setAvailable(true);
    
    return true;
}

bool LoanRepository::updateLoan(const Loan& loan) {
    auto it = std::find_if(loans.begin(), loans.end(), [&loan](const Loan& l) {
        return l.getId() == loan.getId();
    });
    
    if (it != loans.end()) {
        *it = loan;
        return true;
    }
    
    return false;
}

void LoanRepository::checkOverdueLoans() {
    time_t now = time(nullptr);
    
    for (auto& loan : loans) {
        if (loan.getStatus() == Loan::Status::ACTIVE && loan.getDueDate() < now) {
            loan.setStatus(Loan::Status::OVERDUE);
        }
    }
}
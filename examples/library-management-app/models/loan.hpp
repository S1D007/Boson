#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include "../include/external/json.hpp"

class Loan {
public:
    enum class Status {
        ACTIVE,
        RETURNED,
        OVERDUE
    };
    
    Loan() = default;
    Loan(int id, int userId, int bookId, time_t dueDate, time_t checkoutDate = time(nullptr), 
         Status status = Status::ACTIVE, time_t returnDate = 0);
    
    int getId() const;
    int getUserId() const;
    int getBookId() const;
    time_t getDueDate() const;
    time_t getCheckoutDate() const;
    time_t getReturnDate() const;
    Status getStatus() const;
    
    void setReturnDate(time_t date);
    void setStatus(Status status);
    
    nlohmann::json toJson() const;
    static Loan fromJson(const nlohmann::json& json);
    
private:
    int id;
    int userId;
    int bookId;
    time_t dueDate;
    time_t checkoutDate;
    time_t returnDate;
    Status status;
};

class LoanRepository {
public:
    static LoanRepository& getInstance();
    
    std::vector<Loan> getAllLoans() const;
    std::vector<Loan> getLoansByUserId(int userId) const;
    std::vector<Loan> getActiveLoans() const;
    std::vector<Loan> getOverdueLoans() const;
    Loan* getLoanById(int id);
    const Loan* getLoanById(int id) const;
    Loan* getActiveLoanForBook(int bookId);
    
    Loan checkoutBook(int userId, int bookId, int daysToKeep = 14);
    bool returnBook(int loanId);
    bool updateLoan(const Loan& loan);
    void checkOverdueLoans();
    
private:
    LoanRepository();
    std::vector<Loan> loans;
    int nextId = 1;
};
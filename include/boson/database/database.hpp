#ifndef BOSON_DATABASE_HPP
#define BOSON_DATABASE_HPP

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <any>
#include <functional>

namespace boson {
namespace db {

/**
 * @class Connection
 * @brief Abstract base class for database connections
 */
class Connection {
public:
    virtual ~Connection() = default;
    
    /**
     * @brief Open a connection to the database
     * @param connectionString The connection string
     * @return True if the connection was opened successfully, false otherwise
     */
    virtual bool open(const std::string& connectionString) = 0;
    
    /**
     * @brief Close the connection to the database
     * @return True if the connection was closed successfully, false otherwise
     */
    virtual bool close() = 0;
    
    /**
     * @brief Check if the connection is open
     * @return True if the connection is open, false otherwise
     */
    virtual bool isOpen() const = 0;
    
    /**
     * @brief Begin a transaction
     * @return True if the transaction was started successfully, false otherwise
     */
    virtual bool beginTransaction() = 0;
    
    /**
     * @brief Commit a transaction
     * @return True if the transaction was committed successfully, false otherwise
     */
    virtual bool commitTransaction() = 0;
    
    /**
     * @brief Rollback a transaction
     * @return True if the transaction was rolled back successfully, false otherwise
     */
    virtual bool rollbackTransaction() = 0;
};

/**
 * @class QueryResult
 * @brief Abstract base class for query results
 */
class QueryResult {
public:
    virtual ~QueryResult() = default;
    
    /**
     * @brief Check if there are more rows
     * @return True if there are more rows, false otherwise
     */
    virtual bool hasNext() const = 0;
    
    /**
     * @brief Move to the next row
     * @return True if there was a next row, false otherwise
     */
    virtual bool next() = 0;
    
    /**
     * @brief Get the value of a column
     * @param column The column name
     * @return The value of the column
     */
    virtual std::any getValue(const std::string& column) const = 0;
    
    /**
     * @brief Get the value of a column as a specific type
     * @tparam T The type to convert to
     * @param column The column name
     * @return The value of the column as the specified type
     */
    template<typename T>
    T getAs(const std::string& column) const {
        return std::any_cast<T>(getValue(column));
    }
    
    /**
     * @brief Get all values in the current row
     * @return A map of column names to values
     */
    virtual std::map<std::string, std::any> getRow() const = 0;
    
    /**
     * @brief Get all rows
     * @return A vector of maps of column names to values
     */
    virtual std::vector<std::map<std::string, std::any>> getAllRows() = 0;
    
    /**
     * @brief Get the number of rows affected by the query
     * @return The number of rows affected
     */
    virtual int getAffectedRows() const = 0;
    
    /**
     * @brief Get the last inserted ID
     * @return The last inserted ID
     */
    virtual int64_t getLastInsertId() const = 0;
};

/**
 * @class Statement
 * @brief Abstract base class for prepared statements
 */
class Statement {
public:
    virtual ~Statement() = default;
    
    /**
     * @brief Prepare a SQL statement
     * @param sql The SQL statement
     * @return True if the statement was prepared successfully, false otherwise
     */
    virtual bool prepare(const std::string& sql) = 0;
    
    /**
     * @brief Bind a parameter
     * @param index The parameter index (0-based)
     * @param value The value to bind
     * @return True if the parameter was bound successfully, false otherwise
     */
    virtual bool bindParam(int index, const std::any& value) = 0;
    
    /**
     * @brief Execute the statement
     * @return A shared pointer to the query result
     */
    virtual std::shared_ptr<QueryResult> execute() = 0;
};

/**
 * @class Database
 * @brief Abstract base class for databases
 */
class Database {
public:
    virtual ~Database() = default;
    
    /**
     * @brief Create a connection to the database
     * @return A shared pointer to the connection
     */
    virtual std::shared_ptr<Connection> createConnection() = 0;
    
    /**
     * @brief Create a prepared statement
     * @param connection The connection to use
     * @return A shared pointer to the statement
     */
    virtual std::shared_ptr<Statement> createStatement(std::shared_ptr<Connection> connection) = 0;
    
    /**
     * @brief Execute a SQL query
     * @param connection The connection to use
     * @param sql The SQL query
     * @return A shared pointer to the query result
     */
    virtual std::shared_ptr<QueryResult> executeQuery(std::shared_ptr<Connection> connection, const std::string& sql) = 0;
    
    /**
     * @brief Execute a SQL statement
     * @param connection The connection to use
     * @param sql The SQL statement
     * @return The number of rows affected
     */
    virtual int executeUpdate(std::shared_ptr<Connection> connection, const std::string& sql) = 0;
};

/**
 * @class DatabaseFactory
 * @brief Factory for creating database instances
 */
class DatabaseFactory {
public:
    /**
     * @brief Register a database implementation
     * @param name The name of the implementation
     * @param creator A function that creates a new database instance
     */
    static void registerImplementation(const std::string& name, std::function<std::shared_ptr<Database>()> creator);
    
    /**
     * @brief Get a database implementation by name
     * @param name The name of the implementation
     * @return A shared pointer to the database
     */
    static std::shared_ptr<Database> getImplementation(const std::string& name);
};

} 
} 

#endif 
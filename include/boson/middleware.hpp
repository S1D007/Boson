#ifndef BOSON_MIDDLEWARE_HPP
#define BOSON_MIDDLEWARE_HPP

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace boson
{

class Request;
class Response;
class NextFunction;

/**
 * @brief Middleware function type
 * @param req The HTTP request
 * @param res The HTTP response
 * @param next The next middleware in the chain
 */
using Middleware = std::function<void(const Request&, Response&, NextFunction&)>;

/**
 * @class NextFunction
 * @brief Represents the next middleware in the chain
 */
class NextFunction
{
  public:
    NextFunction();
    ~NextFunction();

    /**
     * @brief Call the next middleware
     */
    void operator()();

    /**
     * @brief Call the next middleware with an error
     * @param error The error to pass
     */
    void operator()(const std::string& error);

    /**
     * @brief Set the next middleware
     * @param middleware The next middleware function
     */
    void setNext(const Middleware& middleware);

    /**
     * @brief Set the request and response objects for this middleware
     * @param req The HTTP request
     * @param res The HTTP response
     */
    void setRequestResponse(const Request& req, Response& res);

    /**
     * @brief Check if there is a next middleware
     * @return True if there is a next middleware, false otherwise
     */
    bool hasNext() const;

  private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

/**
 * @struct MiddlewareEntry
 * @brief Stores middleware with optional path pattern
 */
struct MiddlewareEntry {
    Middleware middleware;
    std::optional<std::string> path;
    
    MiddlewareEntry(const Middleware& mw, const std::optional<std::string>& p = std::nullopt) 
        : middleware(mw), path(p) {}
};

/**
 * @class MiddlewareChain
 * @brief Manages a chain of middleware
 */
class MiddlewareChain
{
  public:
    MiddlewareChain();
    ~MiddlewareChain();

    /**
     * @brief Add middleware to the chain
     * @param middleware The middleware to add
     */
    void add(const Middleware& middleware);
    
    /**
     * @brief Add middleware to the chain for a specific path
     * @param path The path pattern to match
     * @param middleware The middleware to add
     */
    void add(const std::string& path, const Middleware& middleware);

    /**
     * @brief Execute the middleware chain
     * @param req The HTTP request
     * @param res The HTTP response
     * @return True if the chain was executed completely, false otherwise
     */
    bool execute(const Request& req, Response& res);
    
    /**
     * @brief Check if path pattern matches the request path
     * @param pattern The path pattern
     * @param path The request path
     * @return True if pattern matches path, false otherwise
     */
    static bool pathMatches(const std::string& pattern, const std::string& path);

  private:
    std::vector<MiddlewareEntry> chain;
};

} // namespace boson

#endif
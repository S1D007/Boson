#ifndef BOSON_MIDDLEWARE_HPP
#define BOSON_MIDDLEWARE_HPP

#include <functional>
#include <memory>
#include <string>
#include <vector>

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
     * @brief Check if there is a next middleware
     * @return True if there is a next middleware, false otherwise
     */
    bool hasNext() const;

  private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
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
     * @brief Execute the middleware chain
     * @param req The HTTP request
     * @param res The HTTP response
     * @return True if the chain was executed completely, false otherwise
     */
    bool execute(const Request& req, Response& res);

  private:
    std::vector<Middleware> chain;
};

} // namespace boson

#endif
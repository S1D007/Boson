#ifndef BOSON_ERROR_HANDLER_HPP
#define BOSON_ERROR_HANDLER_HPP

#include <string>
#include <functional>
#include <memory>
#include <exception>
#include <stdexcept>
#include "request.hpp"
#include "response.hpp"

namespace boson {

/**
 * @class HttpError
 * @brief Base class for HTTP errors
 */
class HttpError : public std::runtime_error {
public:
    explicit HttpError(const std::string& message, int statusCode = 500);
    virtual ~HttpError() = default;
    
    /**
     * @brief Get the HTTP status code
     * @return The HTTP status code
     */
    int statusCode() const;
    
private:
    int code;
};

/**
 * @class BadRequestError
 * @brief Error for 400 Bad Request
 */
class BadRequestError : public HttpError {
public:
    explicit BadRequestError(const std::string& message);
};

/**
 * @class UnauthorizedError
 * @brief Error for 401 Unauthorized
 */
class UnauthorizedError : public HttpError {
public:
    explicit UnauthorizedError(const std::string& message);
};

/**
 * @class ForbiddenError
 * @brief Error for 403 Forbidden
 */
class ForbiddenError : public HttpError {
public:
    explicit ForbiddenError(const std::string& message);
};

/**
 * @class NotFoundError
 * @brief Error for 404 Not Found
 */
class NotFoundError : public HttpError {
public:
    explicit NotFoundError(const std::string& message);
};

/**
 * @brief Error handler function type
 * @param err The error
 * @param req The HTTP request
 * @param res The HTTP response
 */
using ErrorHandler = std::function<void(const std::exception&, const Request&, Response&)>;

/**
 * @brief Default error handler
 * @param err The error
 * @param req The HTTP request
 * @param res The HTTP response
 */
void defaultErrorHandler(const std::exception& err, const Request& req, Response& res);

} 

#endif 
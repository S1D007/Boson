#include "boson/error_handler.hpp"
#include "boson/request.hpp"
#include "boson/response.hpp"

#include <string>
#include <iostream>
#include <exception>

namespace boson {

HttpError::HttpError(const std::string& message, int statusCode)
    : std::runtime_error(message), code(statusCode) {
}

int HttpError::statusCode() const {
    return code;
}

BadRequestError::BadRequestError(const std::string& message)
    : HttpError(message, 400) {
}

UnauthorizedError::UnauthorizedError(const std::string& message)
    : HttpError(message, 401) {
}

ForbiddenError::ForbiddenError(const std::string& message)
    : HttpError(message, 403) {
}

NotFoundError::NotFoundError(const std::string& message)
    : HttpError(message, 404) {
}

void defaultErrorHandler(const std::exception& err, const Request& req, Response& res) {
    
    int statusCode = 500;
    std::string errorMessage = err.what();
    
    const HttpError* httpError = dynamic_cast<const HttpError*>(&err);
    if (httpError) {
        statusCode = httpError->statusCode();
    }
    
    
    std::cerr << "Error: " << errorMessage << " [" << statusCode << "]" << std::endl;
    
    
    std::string jsonResponse = "{\n";
    jsonResponse += "  \"error\": {\n";
    jsonResponse += "    \"status\": " + std::to_string(statusCode) + ",\n";
    jsonResponse += "    \"message\": \"" + errorMessage + "\"\n";
    jsonResponse += "  }\n";
    jsonResponse += "}";
    
    
    res.status(statusCode)
       .header("Content-Type", "application/json")
       .send(jsonResponse);
}

} 
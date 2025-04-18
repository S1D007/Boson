#ifndef BOSON_REQUEST_HPP
#define BOSON_REQUEST_HPP

#include "../external/json.hpp"
#include <any>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <fstream>

namespace boson
{

struct UploadedFile {
    std::string fieldName;
    std::string fileName;
    std::string contentType;
    size_t size;
    std::vector<unsigned char> data;

    bool saveTo(const std::string& path) const {
        std::ofstream out(path, std::ios::binary);
        if (!out) return false;
        out.write(reinterpret_cast<const char*>(data.data()), data.size());
        return out.good();
    }
};

/**
 * @class Request
 * @brief Represents an HTTP request
 */
class Request
{
  public:
    Request();
    ~Request();

    /**
     * @brief Get the HTTP method
     * @return The HTTP method
     */
    std::string method() const;

    /**
     * @brief Get the request path
     * @return The request path
     */
    std::string path() const;

    /**
     * @brief Get the query string
     * @return The query string
     */
    std::string queryString() const;

    /**
     * @brief Get a specific query parameter
     * @param name The name of the query parameter
     * @return The value of the query parameter
     */
    std::string query(const std::string& name) const;

    /**
     * @brief Get all query parameters
     * @return A map of query parameters
     */
    std::map<std::string, std::string> queryParams() const;

    /**
     * @brief Get a specific route parameter
     * @param name The name of the route parameter
     * @return The value of the route parameter
     */
    std::string param(const std::string& name) const;

    /**
     * @brief Get all route parameters
     * @return A map of route parameters
     */
    std::map<std::string, std::string> params() const;

    /**
     * @brief Get a specific header
     * @param name The name of the header
     * @return The value of the header
     */
    std::string header(const std::string& name) const;

    /**
     * @brief Get all headers
     * @return A map of headers
     */
    std::map<std::string, std::string> headers() const;

    /**
     * @brief Get the request body as text
     * @return The request body
     */
    std::string body() const;

    /**
     * @brief Set the request body directly
     * @param body The raw request body
     */
    void setBody(const std::string& body);

    /**
     * @brief Get the request body as JSON
     * @return The request body as a JSON object
     */
    nlohmann::json json() const;

    /**
     * @brief Get a custom property
     * @param name The name of the property
     * @return The value of the property
     */
    std::any get(const std::string& name) const;

    /**
     * @brief Set a custom property
     * @param name The name of the property
     * @param value The value of the property
     */
    void set(const std::string& name, std::any value);

    /**
     * @brief Check if a custom property exists
     * @param name The name of the property
     * @return True if the property exists, false otherwise
     */
    bool has(const std::string& name) const;

    /**
     * @brief Get the client IP address
     * @return The client IP address
     */
    std::string ip() const;

    /**
     * @brief Set the raw HTTP request
     * @param rawRequest The raw HTTP request
     */
    void setRawRequest(const std::string& rawRequest);

    /**
     * @brief Set a route parameter
     * @param name The name of the parameter
     * @param value The value of the parameter
     */
    void setRouteParam(const std::string& name, const std::string& value);

    /**
     * @brief Set the original request path (for internal routing use)
     * @param path The original path
     */
    void setOriginalPath(const std::string& path);

    /**
     * @brief Temporarily override the request path (for internal routing use)
     * @param path The new path to use
     */
    void overridePath(const std::string& path);

    /**
     * @brief Parse the raw HTTP request
     */
    void parse();

    /**
     * @brief Get uploaded files (multipart/form-data)
     * @return Vector of UploadedFile
     */
    std::vector<UploadedFile> files() const;

  private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace boson

#endif
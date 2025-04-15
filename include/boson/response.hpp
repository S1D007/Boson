#ifndef BOSON_RESPONSE_HPP
#define BOSON_RESPONSE_HPP

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <any>
#include <initializer_list>
#include "../external/json.hpp"

namespace boson {

/**
 * @class Response
 * @brief Represents an HTTP response
 */
class Response {
public:
    Response();
    ~Response();

    /**
     * @brief Send a text response
     * @param body The body to send
     * @return Reference to this response for method chaining
     */
    Response& send(const std::string& body);

    /**
     * @brief Send a JSON response
     * @param json The JSON data to send (as std::any)
     * @return Reference to this response for method chaining
     */
    Response& json(const std::any& json);
    
    /**
     * @brief Send a JSON response from a nlohmann::json object
     * @param jsonObj The nlohmann::json object to send
     * @return Reference to this response for method chaining
     */
    Response& jsonObject(const nlohmann::json& jsonObj);
    
    /**
     * @brief Send a JSON response using a JavaScript-like object notation
     * @param items List of key-value pairs forming a JSON object
     * @return Reference to this response for method chaining
     */
    Response& json(std::initializer_list<std::pair<std::string, nlohmann::json>> items);
    
    /**
     * @brief Send a JSON array response
     * @param items List of values forming a JSON array
     * @return Reference to this response for method chaining
     */
    Response& jsonArray(std::initializer_list<nlohmann::json> items);

    /**
     * @brief Set the status code
     * @param code The status code
     * @return Reference to this response for method chaining
     */
    Response& status(int code);

    /**
     * @brief Set a header
     * @param name The name of the header
     * @param value The value of the header
     * @return Reference to this response for method chaining
     */
    Response& header(const std::string& name, const std::string& value);

    /**
     * @brief Set multiple headers
     * @param headers The headers to set
     * @return Reference to this response for method chaining
     */
    Response& headers(const std::map<std::string, std::string>& headers);

    /**
     * @brief Set the content type
     * @param type The content type
     * @return Reference to this response for method chaining
     */
    Response& type(const std::string& type);

    /**
     * @brief Redirect to a different URL
     * @param url The URL to redirect to
     * @param code The status code (default: 302)
     * @return Reference to this response for method chaining
     */
    Response& redirect(const std::string& url, int code = 302);

    /**
     * @brief Check if the response has been sent
     * @return True if the response has been sent, false otherwise
     */
    bool sent() const;

    /**
     * @brief Get the raw HTTP response
     * @return The raw HTTP response
     */
    std::string getRawResponse() const;

    /**
     * @brief Get the status code
     * @return The status code
     */
    int getStatusCode() const;

    /**
     * @brief Get all headers
     * @return A map of headers
     */
    std::map<std::string, std::string> getHeaders() const;

    /**
     * @brief Get the response body
     * @return The response body
     */
    std::string getBody() const;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

} 

#endif
#ifndef BOSON_RESPONSE_HPP
#define BOSON_RESPONSE_HPP

#include "../external/json.hpp"
#include "cookie.hpp"
#include <any>
#include <initializer_list>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <functional>

namespace boson
{

/**
 * @class Response
 * @brief Represents an HTTP response
 */
class Response
{
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
     * @brief Send a file as the response
     * @param path Path to the file to send
     * @param options Optional map containing options for the file response
     * @return Reference to this response for method chaining
     */
    Response& sendFile(const std::string& path, 
                      const std::map<std::string, std::any>& options = {});

    /**
     * @brief Send a file as a downloadable attachment
     * @param path Path to the file to send
     * @param filename Optional filename to use for the download (default: original filename)
     * @param options Optional map containing additional options for the file response
     * @return Reference to this response for method chaining
     */
    Response& download(const std::string& path, const std::string& filename = "",
                      const std::map<std::string, std::any>& options = {});

    /**
     * @brief Stream a file as the response
     * @param path Path to the file to stream
     * @param options Optional map containing options for the file streaming
     * @return Reference to this response for method chaining
     */
    Response& streamFile(const std::string& path,
                        const std::map<std::string, std::any>& options = {});

    /**
     * @brief Enable/disable streaming mode for large responses
     * @param enable Whether to enable streaming
     * @return Reference to this response for method chaining
     */
    Response& stream(bool enable = true);

    /**
     * @brief Write a chunk of data in streaming mode
     * @param chunk The data chunk to write
     * @return Reference to this response for method chaining
     */
    Response& write(const std::string& chunk);

    /**
     * @brief End the streaming response
     * @return Reference to this response for method chaining
     */
    Response& end();

    /**
     * @brief Enable/disable compression for the response
     * @param enable Whether to enable compression
     * @return Reference to this response for method chaining
     */
    Response& compress(bool enable = true);

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
     * @brief Set a cookie
     * @param name The name of the cookie
     * @param value The value of the cookie
     * @return Reference to this response for method chaining
     */
    Response& cookie(const std::string& name, const std::string& value);
    
    /**
     * @brief Set a cookie with options
     * @param name The name of the cookie
     * @param value The value of the cookie
     * @param options Cookie settings (domain, path, maxAge, secure, httpOnly)
     * @return Reference to this response for method chaining
     */
    Response& cookie(const std::string& name, const std::string& value, 
                     const std::map<std::string, std::any>& options);
    
    /**
     * @brief Set a cookie using a Cookie object
     * @param cookie The cookie object to set
     * @return Reference to this response for method chaining
     */
    Response& cookie(const Cookie& cookie);
    
    /**
     * @brief Clear a cookie
     * @param name The name of the cookie to clear
     * @return Reference to this response for method chaining
     */
    Response& clearCookie(const std::string& name);
    
    /**
     * @brief Clear a cookie with options
     * @param name The name of the cookie to clear
     * @param options Cookie settings (domain, path)
     * @return Reference to this response for method chaining
     */
    Response& clearCookie(const std::string& name, 
                          const std::map<std::string, std::string>& options);

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

    /**
     * @brief Set the stream callback function for handling streaming responses
     * @param callback The callback function that takes a string chunk and sends it
     * @return Reference to this response for method chaining
     */
    Response& setStreamCallback(std::function<void(const std::string&)> callback);

  private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
    
    /**
     * @brief Detect MIME type based on file extension
     * @param path Path to the file
     * @return MIME type string
     */
    std::string detectMimeType(const std::string& path) const;
};

} // namespace boson

#endif
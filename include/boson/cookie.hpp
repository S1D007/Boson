#ifndef BOSON_COOKIE_HPP
#define BOSON_COOKIE_HPP

#include <chrono>
#include <map>
#include <optional>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace boson
{

/**
 * @class Cookie
 * @brief Represents an HTTP cookie with attributes
 */
class Cookie
{
  public:
    /**
     * @brief Default constructor
     */
    Cookie() = default;

    /**
     * @brief Constructor with name and value
     * @param name The name of the cookie
     * @param value The value of the cookie
     */
    Cookie(const std::string& name, const std::string& value)
        : name_(name), value_(value) {}

    /**
     * @brief Get the name of the cookie
     * @return The cookie name
     */
    const std::string& name() const { return name_; }
    
    /**
     * @brief Get the value of the cookie
     * @return The cookie value
     */
    const std::string& value() const { return value_; }

    /**
     * @brief Set the domain for the cookie
     * @param domain The domain
     * @return Reference to this cookie for method chaining
     */
    Cookie& domain(const std::string& domain) {
        domain_ = domain;
        return *this;
    }
    
    /**
     * @brief Get the domain
     * @return The domain or empty optional if not set
     */
    const std::optional<std::string>& domain() const { return domain_; }

    /**
     * @brief Set the path for the cookie
     * @param path The path
     * @return Reference to this cookie for method chaining
     */
    Cookie& path(const std::string& path) {
        path_ = path;
        return *this;
    }
    
    /**
     * @brief Get the path
     * @return The path or empty optional if not set
     */
    const std::optional<std::string>& path() const { return path_; }

    /**
     * @brief Set the expiration time
     * @param expires The expiration time as a time_point
     * @return Reference to this cookie for method chaining
     */
    Cookie& expires(const std::chrono::system_clock::time_point& expires) {
        expires_ = expires;
        return *this;
    }
    
    /**
     * @brief Set the expiration time in seconds from now
     * @param seconds Seconds from now
     * @return Reference to this cookie for method chaining
     */
    Cookie& maxAge(int seconds) {
        max_age_ = seconds;
        return *this;
    }
    
    /**
     * @brief Get the max-age value
     * @return The max-age value or empty optional if not set
     */
    const std::optional<int>& maxAge() const { return max_age_; }

    /**
     * @brief Set the cookie to be secure (sent only over HTTPS)
     * @param secure Whether the cookie is secure
     * @return Reference to this cookie for method chaining
     */
    Cookie& secure(bool secure = true) {
        secure_ = secure;
        return *this;
    }
    
    /**
     * @brief Check if the cookie is secure
     * @return True if secure, false otherwise
     */
    bool secure() const { return secure_; }

    /**
     * @brief Set the cookie to be HTTP-only
     * @param httpOnly Whether the cookie is HTTP-only
     * @return Reference to this cookie for method chaining
     */
    Cookie& httpOnly(bool httpOnly = true) {
        http_only_ = httpOnly;
        return *this;
    }
    
    /**
     * @brief Check if the cookie is HTTP-only
     * @return True if HTTP-only, false otherwise
     */
    bool httpOnly() const { return http_only_; }
    
    /**
     * @brief Set the SameSite attribute
     * @param sameSite The SameSite value ("Strict", "Lax", or "None")
     * @return Reference to this cookie for method chaining
     */
    Cookie& sameSite(const std::string& sameSite) {
        same_site_ = sameSite;
        return *this;
    }
    
    /**
     * @brief Get the SameSite attribute
     * @return The SameSite value or empty optional if not set
     */
    const std::optional<std::string>& sameSite() const { return same_site_; }

    /**
     * @brief Convert the cookie to a string representation for a Set-Cookie header
     * @return The cookie as a string
     */
    std::string toString() const {
        std::string result = name_ + "=" + value_;
        
        if (domain_.has_value()) {
            result += "; Domain=" + domain_.value();
        }
        
        if (path_.has_value()) {
            result += "; Path=" + path_.value();
        }
        
        if (expires_.has_value()) {
            auto time_t = std::chrono::system_clock::to_time_t(expires_.value());
            std::stringstream ss;
            ss << std::put_time(std::gmtime(&time_t), "%a, %d %b %Y %H:%M:%S GMT");
            result += "; Expires=" + ss.str();
        }
        
        if (max_age_.has_value()) {
            result += "; Max-Age=" + std::to_string(max_age_.value());
        }
        
        if (secure_) {
            result += "; Secure";
        }
        
        if (http_only_) {
            result += "; HttpOnly";
        }
        
        if (same_site_.has_value()) {
            result += "; SameSite=" + same_site_.value();
        }
        
        return result;
    }

  private:
    std::string name_;
    std::string value_;
    std::optional<std::string> domain_;
    std::optional<std::string> path_;
    std::optional<std::chrono::system_clock::time_point> expires_;
    std::optional<int> max_age_;
    bool secure_ = false;
    bool http_only_ = false;
    std::optional<std::string> same_site_;
};

} // namespace boson

#endif // BOSON_COOKIE_HPP
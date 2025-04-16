#pragma once

#include <boson/middleware.hpp>
#include <boson/request.hpp>
#include <boson/response.hpp>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

class LoggerMiddleware : public boson::Middleware {
public:
    LoggerMiddleware() = default;
    ~LoggerMiddleware() = default;
    
    void process(const boson::Request& req, boson::Response& res, std::function<void()>& next) {
        auto start = std::chrono::steady_clock::now();
        
        // Log request details
        auto timestamp = getCurrentTimeString();
        std::cout << "\033[1;36m[" << timestamp << "]\033[0m ";
        std::cout << "\033[1;33m" << req.method() << "\033[0m ";
        std::cout << "\033[1;32m" << req.path() << "\033[0m";
        
        // Log query params if present
        auto queryParams = req.queryParams();
        if (!queryParams.empty()) {
            std::cout << " - Query: { ";
            bool first = true;
            for (const auto& [key, value] : queryParams) {
                if (!first) std::cout << ", ";
                std::cout << key << ": " << value;
                first = false;
            }
            std::cout << " }";
        }
        
        // Log client IP if available
        std::string clientIP = req.header("X-Forwarded-For");
        if (clientIP.empty()) {
            clientIP = req.header("Remotely-Address");
        }
        if (!clientIP.empty()) {
            std::cout << " - IP: " << clientIP;
        }
        
        std::cout << std::endl;
        
        // Continue to next middleware or route handler
        next();
        
        // Calculate and log response time
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        // Log response details with status code - using a different approach for status information
        int status = 200; // Default status code if not available
        std::cout << "\033[1;36m[" << timestamp << "]\033[0m ";
        std::cout << "\033[1;34mResponse\033[0m ";
        std::cout << "\033[1;35m" << status << "\033[0m ";
        std::cout << "(" << duration << "ms)" << std::endl;
    }
    
private:
    // Helper method to get current time as formatted string
    std::string getCurrentTimeString() {
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        struct tm tm_buf;
        #ifdef _WIN32
        localtime_s(&tm_buf, &now_c);
        #else
        localtime_r(&now_c, &tm_buf);
        #endif
        
        ss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};
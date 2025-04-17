#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <memory>

#include "request.hpp"
#include "response.hpp"
#include "middleware.hpp"

namespace boson {

/**
 * @class StaticFiles
 * @brief Middleware for serving static files from a directory
 * 
 * Similar to Express.js's express.static middleware, this allows
 * easily serving static files from a specified directory.
 */
class StaticFiles {
public:
    /**
     * @brief Create a middleware for serving static files from a directory
     * 
     * @param root Directory path to serve files from
     * @param urlPrefix URL prefix to mount the directory on (default: "/")
     * @param options Additional options for serving (caching, etc.)
     * @return Middleware function that can be passed to Server::use()
     */
    static Middleware create(const std::string& root, const std::string& urlPrefix = "/",
                            const std::unordered_map<std::string, std::string>& options = {}) {
        return [root, urlPrefix, options](const Request& req, Response& res, NextFunction& next) {
            // Only process GET requests
            if (req.method() != "GET") {
                next();
                return;
            }

            // Check if the request path starts with the URL prefix
            std::string path = req.path();
            if (path.rfind(urlPrefix, 0) != 0) {
                next();
                return;
            }

            // Remove the URL prefix from the path to get the relative file path
            std::string relativePath = path.substr(urlPrefix.length());
            
            // Remove leading slash if present
            if (!relativePath.empty() && relativePath[0] == '/') {
                relativePath = relativePath.substr(1);
            }
            
            // Construct the file path
            std::filesystem::path filePath = std::filesystem::path(root) / relativePath;
            
            // Check if file exists and is regular file
            if (!std::filesystem::exists(filePath) || !std::filesystem::is_regular_file(filePath)) {
                next();
                return;
            }

            // Set content type based on file extension
            std::string contentType = getContentType(filePath.extension().string());
            res.header("Content-Type", contentType);
            
            // Set cache headers if cacheControl option is provided
            if (options.find("cacheControl") != options.end()) {
                res.header("Cache-Control", options.at("cacheControl"));
            }

            // Read the file
            std::ifstream file(filePath, std::ios::binary);
            if (!file) {
                next();
                return;
            }

            // Get file size
            std::uintmax_t fileSize = std::filesystem::file_size(filePath);
            
            // Read file content
            std::string content;
            content.resize(fileSize);
            file.read(&content[0], fileSize);
            
            // Send file content
            res.send(content);
        };
    }

private:
    static std::string getContentType(const std::string& extension) {
        static const std::unordered_map<std::string, std::string> contentTypes = {
            {".html", "text/html"},
            {".htm", "text/html"},
            {".css", "text/css"},
            {".js", "application/javascript"},
            {".json", "application/json"},
            {".png", "image/png"},
            {".jpg", "image/jpeg"},
            {".jpeg", "image/jpeg"},
            {".gif", "image/gif"},
            {".svg", "image/svg+xml"},
            {".ico", "image/x-icon"},
            {".txt", "text/plain"},
            {".pdf", "application/pdf"},
            {".xml", "application/xml"},
            {".mp3", "audio/mpeg"},
            {".mp4", "video/mp4"},
            {".woff", "font/woff"},
            {".woff2", "font/woff2"},
            {".ttf", "font/ttf"},
            {".eot", "application/vnd.ms-fontobject"},
            {".otf", "font/otf"},
            {".zip", "application/zip"},
            {".gz", "application/gzip"},
            // Add more MIME types as needed
        };

        auto it = contentTypes.find(extension);
        return it != contentTypes.end() ? it->second : "application/octet-stream";
    }
};

} // namespace boson
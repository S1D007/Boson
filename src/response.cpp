#include "boson/response.hpp"
#include "../include/external/json.hpp"
#include "boson/cookie.hpp"

#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <variant>
#include <vector>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <functional>

namespace boson
{

class Response::Impl
{
  public:
    Impl() : statusCode(200), sentFlag(false), streamingEnabled(false), compressionEnabled(false) {}

    std::map<std::string, std::string> responseHeaders;
    std::string responseBody;
    int statusCode;
    bool sentFlag;
    bool streamingEnabled;
    bool compressionEnabled;
    std::vector<Cookie> cookies;
    std::function<void(const std::string&)> streamCallback;

    std::string getStatusText(int code)
    {
        switch (code)
        {
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 204:
            return "No Content";
        case 206:
            return "Partial Content";
        case 301:
            return "Moved Permanently";
        case 302:
            return "Found";
        case 304:
            return "Not Modified";
        case 307:
            return "Temporary Redirect";
        case 308:
            return "Permanent Redirect";
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 406:
            return "Not Acceptable";
        case 409:
            return "Conflict";
        case 413:
            return "Payload Too Large";
        case 415:
            return "Unsupported Media Type";
        case 416:
            return "Range Not Satisfiable";
        case 422:
            return "Unprocessable Entity";
        case 429:
            return "Too Many Requests";
        case 500:
            return "Internal Server Error";
        case 501:
            return "Not Implemented";
        case 502:
            return "Bad Gateway";
        case 503:
            return "Service Unavailable";
        default:
            return "Unknown";
        }
    }

    std::string buildResponseString()
    {
        std::stringstream ss;

        ss << "HTTP/1.1 " << statusCode << " " << getStatusText(statusCode) << "\r\n";

        if (responseHeaders.find("Content-Type") == responseHeaders.end())
        {
            responseHeaders["Content-Type"] = "text/plain";
        }

        responseHeaders["Content-Length"] = std::to_string(responseBody.length());
        responseHeaders["Connection"] = "close";

        // Add all headers
        for (const auto& header : responseHeaders)
        {
            ss << header.first << ": " << header.second << "\r\n";
        }

        // Add all cookies
        for (const auto& cookie : cookies)
        {
            ss << "Set-Cookie: " << cookie.toString() << "\r\n";
        }

        ss << "\r\n";
        ss << responseBody;

        return ss.str();
    }
};

Response::Response() : pimpl(std::make_unique<Impl>()) {}

Response::~Response() {}

Response& Response::send(const std::string& body)
{
    if (!pimpl->sentFlag)
    {
        pimpl->responseBody = body;
        pimpl->sentFlag = true;
    }
    return *this;
}

Response& Response::json(const std::any& jsonData)
{
    if (!pimpl->sentFlag)
    {
        pimpl->responseHeaders["Content-Type"] = "application/json";

        try
        {
            if (jsonData.type() == typeid(std::string))
            {
                pimpl->responseBody = std::any_cast<std::string>(jsonData);
                pimpl->sentFlag = true;
                return *this;
            }

            nlohmann::json j;

            if (jsonData.type() == typeid(nlohmann::json))
            {
                j = std::any_cast<nlohmann::json>(jsonData);
            }
            else
            {
                j = nlohmann::json::object();
            }

            pimpl->responseBody = j.dump();
        }
        catch (const std::exception& e)
        {
            pimpl->responseBody = "{}";
        }

        pimpl->sentFlag = true;
    }
    return *this;
}

Response& Response::jsonObject(const nlohmann::json& jsonObj)
{
    if (!pimpl->sentFlag)
    {
        pimpl->responseHeaders["Content-Type"] = "application/json";
        pimpl->responseBody = jsonObj.dump();
        pimpl->sentFlag = true;
    }
    return *this;
}

Response& Response::json(std::initializer_list<std::pair<std::string, nlohmann::json>> items)
{
    if (!pimpl->sentFlag)
    {
        pimpl->responseHeaders["Content-Type"] = "application/json";

        nlohmann::json jsonObj;
        for (const auto& item : items)
        {
            jsonObj[item.first] = item.second;
        }

        pimpl->responseBody = jsonObj.dump();
        pimpl->sentFlag = true;
    }
    return *this;
}

Response& Response::jsonArray(std::initializer_list<nlohmann::json> items)
{
    if (!pimpl->sentFlag)
    {
        pimpl->responseHeaders["Content-Type"] = "application/json";

        nlohmann::json jsonArr = nlohmann::json::array();
        for (const auto& item : items)
        {
            jsonArr.push_back(item);
        }

        pimpl->responseBody = jsonArr.dump();
        pimpl->sentFlag = true;
    }
    return *this;
}

Response& Response::status(int code)
{
    pimpl->statusCode = code;
    return *this;
}

Response& Response::header(const std::string& name, const std::string& value)
{
    pimpl->responseHeaders[name] = value;
    return *this;
}

Response& Response::headers(const std::map<std::string, std::string>& headers)
{
    for (const auto& header : headers)
    {
        pimpl->responseHeaders[header.first] = header.second;
    }
    return *this;
}

Response& Response::type(const std::string& type)
{
    return header("Content-Type", type);
}

Response& Response::redirect(const std::string& url, int code)
{
    pimpl->statusCode = code;
    header("Location", url);
    return send("");
}

Response& Response::cookie(const std::string& name, const std::string& value)
{
    Cookie cookie(name, value);
    pimpl->cookies.push_back(cookie);
    return *this;
}

Response& Response::cookie(const std::string& name, const std::string& value,
                          const std::map<std::string, std::any>& options)
{
    Cookie cookie(name, value);

    // Apply cookie options
    for (const auto& option : options)
    {
        if (option.first == "domain" && option.second.type() == typeid(std::string))
        {
            cookie.domain(std::any_cast<std::string>(option.second));
        }
        else if (option.first == "path" && option.second.type() == typeid(std::string))
        {
            cookie.path(std::any_cast<std::string>(option.second));
        }
        else if (option.first == "maxAge" && option.second.type() == typeid(int))
        {
            cookie.maxAge(std::any_cast<int>(option.second));
        }
        else if (option.first == "secure" && option.second.type() == typeid(bool))
        {
            cookie.secure(std::any_cast<bool>(option.second));
        }
        else if (option.first == "httpOnly" && option.second.type() == typeid(bool))
        {
            cookie.httpOnly(std::any_cast<bool>(option.second));
        }
        else if (option.first == "sameSite" && option.second.type() == typeid(std::string))
        {
            cookie.sameSite(std::any_cast<std::string>(option.second));
        }
        else if (option.first == "expires")
        {
            if (option.second.type() == typeid(std::chrono::system_clock::time_point))
            {
                cookie.expires(std::any_cast<std::chrono::system_clock::time_point>(option.second));
            }
        }
    }

    pimpl->cookies.push_back(cookie);
    return *this;
}

Response& Response::cookie(const Cookie& cookie)
{
    pimpl->cookies.push_back(cookie);
    return *this;
}

Response& Response::clearCookie(const std::string& name)
{
    Cookie cookie(name, "");
    cookie.maxAge(0);
    pimpl->cookies.push_back(cookie);
    return *this;
}

Response& Response::clearCookie(const std::string& name,
                              const std::map<std::string, std::string>& options)
{
    Cookie cookie(name, "");
    cookie.maxAge(0);

    // Apply path/domain options for proper cookie clearing
    for (const auto& option : options)
    {
        if (option.first == "domain")
        {
            cookie.domain(option.second);
        }
        else if (option.first == "path")
        {
            cookie.path(option.second);
        }
    }

    pimpl->cookies.push_back(cookie);
    return *this;
}

bool Response::sent() const
{
    return pimpl->sentFlag;
}

std::string Response::getRawResponse() const
{
    return pimpl->buildResponseString();
}

int Response::getStatusCode() const
{
    return pimpl->statusCode;
}

std::map<std::string, std::string> Response::getHeaders() const
{
    return pimpl->responseHeaders;
}

std::string Response::getBody() const
{
    return pimpl->responseBody;
}

std::string Response::detectMimeType(const std::string& path) const
{
    static const std::map<std::string, std::string> mimeTypes = {
        // Text
        {".txt", "text/plain"},
        {".html", "text/html"},
        {".htm", "text/html"},
        {".css", "text/css"},
        {".csv", "text/csv"},
        {".md", "text/markdown"},
        
        // Application
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".xml", "application/xml"},
        {".pdf", "application/pdf"},
        {".zip", "application/zip"},
        {".gz", "application/gzip"},
        {".tar", "application/x-tar"},
        
        // Images
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".svg", "image/svg+xml"},
        {".webp", "image/webp"},
        {".ico", "image/x-icon"},
        {".bmp", "image/bmp"},
        {".tiff", "image/tiff"},
        {".tif", "image/tiff"},
        
        // Audio
        {".mp3", "audio/mpeg"},
        {".wav", "audio/wav"},
        {".ogg", "audio/ogg"},
        {".flac", "audio/flac"},
        {".aac", "audio/aac"},
        
        // Video
        {".mp4", "video/mp4"},
        {".mpeg", "video/mpeg"},
        {".webm", "video/webm"},
        {".avi", "video/x-msvideo"},
        {".mov", "video/quicktime"},
        
        // Fonts
        {".woff", "font/woff"},
        {".woff2", "font/woff2"},
        {".ttf", "font/ttf"},
        {".otf", "font/otf"},
        {".eot", "application/vnd.ms-fontobject"},
        
        // MS Office
        {".doc", "application/msword"},
        {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
        {".xls", "application/vnd.ms-excel"},
        {".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
        {".ppt", "application/vnd.ms-powerpoint"},
        {".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
        
        // Code
        {".c", "text/x-c"},
        {".cpp", "text/x-c++"},
        {".h", "text/x-c"},
        {".hpp", "text/x-c++"},
        {".py", "text/x-python"},
        {".java", "text/x-java"},
        {".sh", "text/x-shellscript"},
        {".rb", "text/x-ruby"},
        {".go", "text/x-go"},
        {".php", "text/x-php"},
        {".ts", "application/typescript"},

        // Other common types
        {".bin", "application/octet-stream"},
        {".exe", "application/octet-stream"}
    };
    
    try {
        std::filesystem::path filePath(path);
        std::string extension = filePath.extension().string();
        
        std::transform(extension.begin(), extension.end(), extension.begin(),
                      [](unsigned char c){ return std::tolower(c); });
        
        auto it = mimeTypes.find(extension);
        return it != mimeTypes.end() ? it->second : "application/octet-stream";
    } catch (const std::exception& e) {
        return "application/octet-stream";
    }
}

Response& Response::sendFile(const std::string& path, const std::map<std::string, std::any>& options)
{
    if (pimpl->sentFlag) {
        return *this;
    }
    
    try {
        std::filesystem::path filePath(path);
        
        if (!std::filesystem::exists(filePath)) {
            status(404);
            return send("File not found");
        }
        
        if (!std::filesystem::is_regular_file(filePath)) {
            status(403);
            return send("Not a file");
        }
        
        std::string contentType = detectMimeType(path);
        if (pimpl->responseHeaders.find("Content-Type") == pimpl->responseHeaders.end()) {
            header("Content-Type", contentType);
        }
        
        if (options.find("cacheControl") != options.end() && options.at("cacheControl").type() == typeid(std::string)) {
            header("Cache-Control", std::any_cast<std::string>(options.at("cacheControl")));
        }
        
        if (options.find("etag") != options.end() && options.at("etag").type() == typeid(std::string)) {
            header("ETag", std::any_cast<std::string>(options.at("etag")));
        } else {
            std::string etag = "\"" + 
                std::to_string(static_cast<long long>(std::filesystem::last_write_time(filePath).time_since_epoch().count())) + 
                "-" + 
                std::to_string(static_cast<uintmax_t>(std::filesystem::file_size(filePath))) + 
                "\"";
            header("ETag", etag);
        }
        
        auto lastModified = std::filesystem::last_write_time(filePath);
        auto fsEpochDuration = lastModified.time_since_epoch();
        auto sysTime = std::chrono::system_clock::time_point(
            std::chrono::duration_cast<std::chrono::system_clock::duration>(fsEpochDuration)
        );
        auto lastModifiedTime = std::chrono::system_clock::to_time_t(sysTime);
        
        char lastModifiedStr[100];
        std::strftime(lastModifiedStr, sizeof(lastModifiedStr), 
                     "%a, %d %b %Y %H:%M:%S GMT", 
                     std::gmtime(&lastModifiedTime));
        header("Last-Modified", lastModifiedStr);
        
        bool useStreaming = false;
        if (options.find("stream") != options.end() && options.at("stream").type() == typeid(bool)) {
            useStreaming = std::any_cast<bool>(options.at("stream"));
        } else {
            auto fileSize = std::filesystem::file_size(filePath);
            useStreaming = (fileSize > 1024 * 1024);
        }
        
        if (useStreaming && pimpl->streamCallback) {
            return streamFile(path, options);
        }
        
        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            status(500);
            return send("Failed to read file");
        }
        
        file.seekg(0, std::ios::end);
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::string content;
        content.resize(fileSize);
        
        if (file.read(&content[0], fileSize)) {
            pimpl->sentFlag = true;
            pimpl->responseBody = content;
        } else {
            status(500);
            return send("Error reading file");
        }
    } catch (const std::exception& e) {
        status(500);
        return send("Internal server error: " + std::string(e.what()));
    }
    
    return *this;
}

Response& Response::download(const std::string& path, const std::string& filename,
                          const std::map<std::string, std::any>& options)
{
    if (pimpl->sentFlag) {
        return *this;
    }
    
    std::string dispositionFilename = filename.empty() ? 
        std::filesystem::path(path).filename().string() : 
        filename;
    
    std::string encodedFilename;
    for (char c : dispositionFilename) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encodedFilename += c;
        } else {
            char hex[4];
            std::snprintf(hex, sizeof(hex), "%%%02X", static_cast<unsigned char>(c));
            encodedFilename += hex;
        }
    }
    
    header("Content-Disposition", "attachment; filename=\"" + dispositionFilename + 
           "\"; filename*=UTF-8''" + encodedFilename);
    
    return sendFile(path, options);
}

Response& Response::streamFile(const std::string& path,
                            const std::map<std::string, std::any>& options)
{
    if (pimpl->sentFlag || !pimpl->streamCallback) {
        return *this;
    }
    
    try {
        std::filesystem::path filePath(path);
        
        if (!std::filesystem::exists(filePath)) {
            status(404);
            return send("File not found");
        }
        
        if (!std::filesystem::is_regular_file(filePath)) {
            status(403);
            return send("Not a file");
        }
        
        std::string contentType = detectMimeType(path);
        if (pimpl->responseHeaders.find("Content-Type") == pimpl->responseHeaders.end()) {
            header("Content-Type", contentType);
        }
        
        header("Transfer-Encoding", "chunked");
        
        pimpl->responseHeaders.erase("Content-Length");
        
        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            status(500);
            return send("Failed to read file");
        }
        
        size_t chunkSize = 8192;
        if (options.find("chunkSize") != options.end() && options.at("chunkSize").type() == typeid(size_t)) {
            chunkSize = std::any_cast<size_t>(options.at("chunkSize"));
        }
        
        pimpl->sentFlag = true;
        
        char* buffer = new char[chunkSize];
        
        while (file) {
            file.read(buffer, chunkSize);
            std::streamsize bytesRead = file.gcount();
            
            if (bytesRead > 0) {
                std::stringstream chunkHeader;
                chunkHeader << std::hex << bytesRead << "\r\n";
                
                pimpl->streamCallback(chunkHeader.str());
                
                pimpl->streamCallback(std::string(buffer, bytesRead));
                
                pimpl->streamCallback("\r\n");
            }
            
            if (bytesRead < static_cast<std::streamsize>(chunkSize)) {
                break;
            }
        }
        
        delete[] buffer;
        
        pimpl->streamCallback("0\r\n\r\n");
        
    } catch (const std::exception& e) {
        status(500);
        return send("Internal server error: " + std::string(e.what()));
    }
    
    return *this;
}

Response& Response::stream(bool enable)
{
    pimpl->streamingEnabled = enable;
    return *this;
}

Response& Response::write(const std::string& chunk)
{
    if (pimpl->streamingEnabled && pimpl->streamCallback && !chunk.empty()) {
        std::stringstream chunkHeader;
        chunkHeader << std::hex << chunk.length() << "\r\n";
        
        pimpl->streamCallback(chunkHeader.str());
        pimpl->streamCallback(chunk);
        pimpl->streamCallback("\r\n");
    }
    return *this;
}

Response& Response::end()
{
    if (pimpl->streamingEnabled && pimpl->streamCallback) {
        pimpl->streamCallback("0\r\n\r\n");
        pimpl->sentFlag = true;
    }
    return *this;
}

Response& Response::compress(bool enable)
{
    pimpl->compressionEnabled = enable;
    if (enable) {
        header("Content-Encoding", "gzip");
    } else {
        auto it = pimpl->responseHeaders.find("Content-Encoding");
        if (it != pimpl->responseHeaders.end()) {
            pimpl->responseHeaders.erase(it);
        }
    }
    return *this;
}

Response& Response::setStreamCallback(std::function<void(const std::string&)> callback)
{
    pimpl->streamCallback = callback;
    return *this;
}

} // namespace boson
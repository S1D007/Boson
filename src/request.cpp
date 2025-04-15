#include "boson/request.hpp"
#include "../include/external/json.hpp"

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <sstream>
#include <regex>
#include <algorithm>

namespace boson {

class Request::Impl {
public:
    Impl() {}
    
    std::string rawRequest;
    std::string requestMethod;
    std::string requestPath;
    std::string requestQueryString;
    std::map<std::string, std::string> requestQueryParams;
    std::map<std::string, std::string> requestRouteParams;
    std::map<std::string, std::string> requestHeaders;
    std::string requestBody;
    std::map<std::string, std::any> customProperties;
    std::string clientIP;
    std::string originalRequestPath;
    
    void parseMethod(const std::string& firstLine) {
        std::istringstream iss(firstLine);
        iss >> requestMethod;
    }
    
    void parsePath(const std::string& firstLine) {
        std::istringstream iss(firstLine);
        std::string method;
        iss >> method; 
        
        std::string fullPath;
        iss >> fullPath;
        
        auto queryPos = fullPath.find('?');
        if (queryPos != std::string::npos) {
            requestPath = fullPath.substr(0, queryPos);
            requestQueryString = fullPath.substr(queryPos + 1);
            parseQueryParams();
        } else {
            requestPath = fullPath;
        }
    }
    
    void parseQueryParams() {
        std::istringstream iss(requestQueryString);
        std::string param;
        
        while (std::getline(iss, param, '&')) {
            auto equalsPos = param.find('=');
            if (equalsPos != std::string::npos) {
                std::string key = param.substr(0, equalsPos);
                std::string value = param.substr(equalsPos + 1);
                requestQueryParams[key] = value;
            } else {
                requestQueryParams[param] = "";
            }
        }
    }
    
    void parseHeaders(const std::vector<std::string>& lines) {
        for (size_t i = 1; i < lines.size(); i++) {
            const std::string& line = lines[i];
            
            if (line.empty()) {
                
                break;
            }
            
            auto colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string key = line.substr(0, colonPos);
                
                
                size_t valueStart = colonPos + 1;
                while (valueStart < line.size() && std::isspace(line[valueStart])) {
                    valueStart++;
                }
                
                std::string value = line.substr(valueStart);
                requestHeaders[key] = value;
                
                if (key == "Content-Length") {
                    
                }
            }
        }
    }
    
    void parseBody(const std::vector<std::string>& lines) {
        bool inBody = false;
        std::stringstream bodyStream;
        
        for (const auto& line : lines) {
            if (inBody) {
                bodyStream << line << "\r\n";
            } else if (line.empty()) {
                inBody = true;
            }
        }
        
        requestBody = bodyStream.str();
    }
};

Request::Request() : pimpl(std::make_unique<Impl>()) {
}

Request::~Request() {
}

std::string Request::method() const {
    return pimpl->requestMethod;
}

std::string Request::path() const {
    return pimpl->requestPath;
}

std::string Request::queryString() const {
    return pimpl->requestQueryString;
}

std::string Request::query(const std::string& name) const {
    auto it = pimpl->requestQueryParams.find(name);
    return (it != pimpl->requestQueryParams.end()) ? it->second : "";
}

std::map<std::string, std::string> Request::queryParams() const {
    return pimpl->requestQueryParams;
}

std::string Request::param(const std::string& name) const {
    auto it = pimpl->requestRouteParams.find(name);
    return (it != pimpl->requestRouteParams.end()) ? it->second : "";
}

std::map<std::string, std::string> Request::params() const {
    return pimpl->requestRouteParams;
}

std::string Request::header(const std::string& name) const {
    auto it = pimpl->requestHeaders.find(name);
    return (it != pimpl->requestHeaders.end()) ? it->second : "";
}

std::map<std::string, std::string> Request::headers() const {
    return pimpl->requestHeaders;
}

std::string Request::body() const {
    return pimpl->requestBody;
}

nlohmann::json Request::json() const {
    try {
        
        std::string contentType = header("Content-Type");
        bool isJsonContent = 
            contentType.find("application/json") != std::string::npos || 
            contentType.find("application/problem+json") != std::string::npos;
            
        if (isJsonContent || !pimpl->requestBody.empty()) {
            return nlohmann::json::parse(pimpl->requestBody);
        }
    } catch (const std::exception& e) {
        
    }
    
    return nlohmann::json::object();
}

std::any Request::get(const std::string& name) const {
    auto it = pimpl->customProperties.find(name);
    return (it != pimpl->customProperties.end()) ? it->second : std::any();
}

void Request::set(const std::string& name, std::any value) {
    pimpl->customProperties[name] = value;
}

bool Request::has(const std::string& name) const {
    return pimpl->customProperties.find(name) != pimpl->customProperties.end();
}

std::string Request::ip() const {
    return pimpl->clientIP;
}

void Request::setRawRequest(const std::string& rawRequest) {
    pimpl->rawRequest = rawRequest;
}

void Request::setRouteParam(const std::string& name, const std::string& value) {
    pimpl->requestRouteParams[name] = value;
}

void Request::parse() {
    
    std::vector<std::string> lines;
    std::istringstream iss(pimpl->rawRequest);
    std::string line;
    
    while (std::getline(iss, line)) {
        
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        lines.push_back(line);
    }
    
    if (lines.empty()) {
        return;
    }
    
    
    pimpl->parseMethod(lines[0]);
    pimpl->parsePath(lines[0]);
    
    
    pimpl->parseHeaders(lines);
    pimpl->parseBody(lines);
}

void Request::setOriginalPath(const std::string& path) {
    pimpl->originalRequestPath = path;
}

void Request::overridePath(const std::string& path) {
    pimpl->requestPath = path;
}

}
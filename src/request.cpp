#include "boson/request.hpp"
#include "../include/external/json.hpp"

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <cstring>

namespace boson
{

class Request::Impl
{
public:
    Impl() {}

    const char* rawRequest = nullptr;
    size_t rawRequestLength = 0;
    std::string requestMethod;
    std::string requestPath;
    std::string requestQueryString;
    std::map<std::string, std::string> requestQueryParams;
    std::map<std::string, std::string> requestRouteParams;
    std::map<std::string, std::string> requestHeaders;
    std::string_view requestBodyView;
    std::map<std::string, std::any> customProperties;
    std::string clientIP;
    std::string originalRequestPath;

    static const char* findSubstring(const char* str, size_t strLength, const char* substr) {
        const char* end = str + strLength;
        size_t substrLen = strlen(substr);
        
        if (substrLen > strLength) return nullptr;
        
        for (const char* p = str; p <= end - substrLen; ++p) {
            if (memcmp(p, substr, substrLen) == 0) {
                return p;
            }
        }
        
        return nullptr;
    }

    void parseRequest() {
        if (!rawRequest || rawRequestLength == 0) return;

        const char* lineEnd = findSubstring(rawRequest, rawRequestLength, "\r\n");
        if (!lineEnd) lineEnd = findSubstring(rawRequest, rawRequestLength, "\n");
        if (!lineEnd) return;

        parseMethodAndPath(std::string_view(rawRequest, lineEnd - rawRequest));
        
        const char* headersEnd = findSubstring(rawRequest, rawRequestLength, "\r\n\r\n");
        if (!headersEnd) headersEnd = findSubstring(rawRequest, rawRequestLength, "\n\n");
        
        if (headersEnd) {
            const char* headerStart = lineEnd + (*(lineEnd + 1) == '\n' ? 1 : 2);
            parseHeaders(std::string_view(headerStart, headersEnd - headerStart));
            
            const char* bodyStart = headersEnd + (*(headersEnd + 2) == '\n' ? 2 : 4);
            if (bodyStart < rawRequest + rawRequestLength) {
                requestBodyView = std::string_view(bodyStart, (rawRequest + rawRequestLength) - bodyStart);
            }
        }
    }

    void parseMethodAndPath(std::string_view firstLine) {
        size_t methodEnd = firstLine.find(' ');
        if (methodEnd == std::string_view::npos) return;
        
        requestMethod = std::string(firstLine.substr(0, methodEnd));
        
        size_t pathStart = methodEnd + 1;
        size_t pathEnd = firstLine.find(' ', pathStart);
        if (pathEnd == std::string_view::npos) pathEnd = firstLine.length();
        
        std::string_view fullPath = firstLine.substr(pathStart, pathEnd - pathStart);
        
        size_t queryPos = fullPath.find('?');
        if (queryPos != std::string_view::npos) {
            requestPath = std::string(fullPath.substr(0, queryPos));
            requestQueryString = std::string(fullPath.substr(queryPos + 1));
            parseQueryParams();
        } else {
            requestPath = std::string(fullPath);
        }
    }

    void parseHeaders(std::string_view headersSection) {
        size_t pos = 0;
        size_t lineEnd;
        
        while ((lineEnd = headersSection.find("\r\n", pos)) != std::string_view::npos ||
               (lineEnd = headersSection.find("\n", pos)) != std::string_view::npos) {
            
            std::string_view line = headersSection.substr(pos, lineEnd - pos);
            
            size_t colonPos = line.find(':');
            if (colonPos != std::string_view::npos) {
                std::string key(line.substr(0, colonPos));
                
                size_t valueStart = colonPos + 1;
                while (valueStart < line.size() && std::isspace(line[valueStart])) {
                    valueStart++;
                }
                
                std::string value(line.substr(valueStart));
                requestHeaders[key] = value;
            }
            
            pos = lineEnd + ((headersSection[lineEnd] == '\r') ? 2 : 1);
            if (pos >= headersSection.length()) break;
        }
    }

    void parseQueryParams() {
        size_t pos = 0;
        size_t ampPos;
        
        while ((ampPos = requestQueryString.find('&', pos)) != std::string::npos) {
            std::string param = requestQueryString.substr(pos, ampPos - pos);
            addQueryParam(param);
            pos = ampPos + 1;
        }
        
        if (pos < requestQueryString.length()) {
            addQueryParam(requestQueryString.substr(pos));
        }
    }

    void addQueryParam(const std::string& param) {
        auto equalsPos = param.find('=');
        if (equalsPos != std::string::npos) {
            std::string key = param.substr(0, equalsPos);
            std::string value = param.substr(equalsPos + 1);
            requestQueryParams[key] = value;
        } else {
            requestQueryParams[param] = "";
        }
    }
};

Request::Request() : pimpl(std::make_unique<Impl>()) {}

Request::~Request() {}

std::string Request::method() const
{
    return pimpl->requestMethod;
}

std::string Request::path() const
{
    return pimpl->requestPath;
}

std::string Request::queryString() const
{
    return pimpl->requestQueryString;
}

std::string Request::query(const std::string& name) const
{
    auto it = pimpl->requestQueryParams.find(name);
    return (it != pimpl->requestQueryParams.end()) ? it->second : "";
}

std::map<std::string, std::string> Request::queryParams() const
{
    return pimpl->requestQueryParams;
}

std::string Request::param(const std::string& name) const
{
    auto it = pimpl->requestRouteParams.find(name);
    return (it != pimpl->requestRouteParams.end()) ? it->second : "";
}

std::map<std::string, std::string> Request::params() const
{
    return pimpl->requestRouteParams;
}

std::string Request::header(const std::string& name) const
{
    auto it = pimpl->requestHeaders.find(name);
    return (it != pimpl->requestHeaders.end()) ? it->second : "";
}

std::map<std::string, std::string> Request::headers() const
{
    return pimpl->requestHeaders;
}

std::string Request::body() const
{
    return std::string(pimpl->requestBodyView);
}

nlohmann::json Request::json() const
{
    try
    {
        std::string contentType = header("Content-Type");
        bool isJsonContent = contentType.find("application/json") != std::string::npos ||
                             contentType.find("application/problem+json") != std::string::npos;

        if (isJsonContent || !pimpl->requestBodyView.empty())
        {
            return nlohmann::json::parse(pimpl->requestBodyView);
        }
    }
    catch (const std::exception& e)
    {
    }

    return nlohmann::json::object();
}

std::any Request::get(const std::string& name) const
{
    auto it = pimpl->customProperties.find(name);
    return (it != pimpl->customProperties.end()) ? it->second : std::any();
}

void Request::set(const std::string& name, std::any value)
{
    pimpl->customProperties[name] = value;
}

bool Request::has(const std::string& name) const
{
    return pimpl->customProperties.find(name) != pimpl->customProperties.end();
}

std::string Request::ip() const
{
    return pimpl->clientIP;
}

void Request::setRawRequest(const char* rawRequest)
{
    pimpl->rawRequest = rawRequest;
    pimpl->rawRequestLength = strlen(rawRequest);
}

void Request::setRawRequest(const char* rawRequest, size_t length)
{
    pimpl->rawRequest = rawRequest;
    pimpl->rawRequestLength = length;
}

void Request::setRouteParam(const std::string& name, const std::string& value)
{
    pimpl->requestRouteParams[name] = value;
}

void Request::parse()
{
    pimpl->parseRequest();
}

void Request::setOriginalPath(const std::string& path)
{
    pimpl->originalRequestPath = path;
}

void Request::overridePath(const std::string& path)
{
    pimpl->requestPath = path;
}

} // namespace boson
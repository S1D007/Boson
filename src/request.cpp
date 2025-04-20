#include "boson/request.hpp"
#include "../include/external/json.hpp"
#include <algorithm>
#include <map>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace boson
{

class Request::Impl
{
  public:
    Impl() : isSecure(false) {}

    std::string rawRequest;
    std::string requestMethod;
    std::string requestPath;
    std::string requestQueryString;
    std::string fullUrl;
    std::map<std::string, std::string> requestQueryParams;
    std::map<std::string, std::string> requestRouteParams;
    std::map<std::string, std::string> requestHeaders;
    std::map<std::string, std::string> requestCookies;
    std::string requestBody;
    std::map<std::string, std::any> customProperties;
    std::string clientIP;
    std::string originalRequestPath;
    std::string requestProtocol;
    bool isSecure;
    std::vector<UploadedFile> uploadedFiles;

    void parseMethod(const std::string& firstLine)
    {
        std::istringstream iss(firstLine);
        iss >> requestMethod;
    }

    void parsePath(const std::string& firstLine)
    {
        std::istringstream iss(firstLine);
        std::string method;
        iss >> method;

        std::string fullPath;
        iss >> fullPath;

        fullUrl = fullPath;

        auto queryPos = fullPath.find('?');
        if (queryPos != std::string::npos)
        {
            requestPath = fullPath.substr(0, queryPos);
            requestQueryString = fullPath.substr(queryPos + 1);
            parseQueryParams();
        }
        else
        {
            requestPath = fullPath;
        }
    }

    void parseQueryParams()
    {
        std::istringstream iss(requestQueryString);
        std::string param;

        while (std::getline(iss, param, '&'))
        {
            auto equalsPos = param.find('=');
            if (equalsPos != std::string::npos)
            {
                std::string key = param.substr(0, equalsPos);
                std::string value = param.substr(equalsPos + 1);
                requestQueryParams[key] = value;
            }
            else
            {
                requestQueryParams[param] = "";
            }
        }
    }

    void parseHeaders(const std::vector<std::string>& lines)
    {
        for (size_t i = 1; i < lines.size(); i++)
        {
            const std::string& line = lines[i];

            if (line.empty())
            {
                break;
            }

            auto colonPos = line.find(':');
            if (colonPos != std::string::npos)
            {
                std::string key = line.substr(0, colonPos);

                size_t valueStart = colonPos + 1;
                while (valueStart < line.size() && std::isspace(line[valueStart]))
                {
                    valueStart++;
                }

                std::string value = line.substr(valueStart);
                requestHeaders[key] = value;

                if (key == "X-Forwarded-Proto" && value == "https") {
                    requestProtocol = "https";
                    isSecure = true;
                }
                
                if (key == "Cookie") {
                    parseCookies(value);
                }
            }
        }
        
        if (requestProtocol.empty()) {
            requestProtocol = "http";
        }
    }

    void parseCookies(const std::string& cookieHeader) {
        std::istringstream iss(cookieHeader);
        std::string cookiePair;
        
        std::string cookieStr = cookieHeader;
        size_t pos = 0;
        std::string delimiter = ";";
        
        while ((pos = cookieStr.find(delimiter)) != std::string::npos) {
            std::string token = cookieStr.substr(0, pos);
            parseCookiePair(token);
            cookieStr.erase(0, pos + delimiter.length());
        }
        
        if (!cookieStr.empty()) {
            parseCookiePair(cookieStr);
        }
    }
    
    void parseCookiePair(const std::string& pair) {
        std::string trimmed = pair;
        trimmed.erase(0, trimmed.find_first_not_of(" \t"));
        trimmed.erase(trimmed.find_last_not_of(" \t") + 1);
        
        size_t equalsPos = trimmed.find('=');
        if (equalsPos != std::string::npos) {
            std::string name = trimmed.substr(0, equalsPos);
            std::string value = trimmed.substr(equalsPos + 1);
            requestCookies[name] = value;
        }
    }

    void parseBody(const std::vector<std::string>& lines)
    {
        size_t headerEnd = 0;
        for (size_t i = 0; i < lines.size(); ++i) {
            if (lines[i].empty()) {
                headerEnd = i;
                break;
            }
        }

        if (headerEnd == 0 || headerEnd >= lines.size() - 1) {
            requestBody = "";
            return;
        }

        size_t expectedLength = 0;
        auto it = requestHeaders.find("Content-Length");
        if (it != requestHeaders.end()) {
            try {
                expectedLength = std::stoul(it->second);
            } catch(...) {
            }
        }

        if (!rawRequest.empty()) {
            size_t bodyStart = rawRequest.find("\r\n\r\n");
            if (bodyStart != std::string::npos) {
                bodyStart += 4;
                requestBody = rawRequest.substr(bodyStart);
            }
        } else {
            std::stringstream bodyStream;
            for (size_t i = headerEnd + 1; i < lines.size(); ++i) {
                bodyStream << lines[i] << "\r\n";
            }
            requestBody = bodyStream.str();
        }
        
        auto contentTypeIt = requestHeaders.find("Content-Type");
        if (contentTypeIt != requestHeaders.end() && contentTypeIt->second.find("multipart/form-data") != std::string::npos) {
            std::string contentType = contentTypeIt->second;
            std::string boundary;
            
            std::string::size_type pos = contentType.find("boundary=");
            if (pos != std::string::npos) {
                pos += 9;
                
                if (pos < contentType.length() && contentType[pos] == '"') {
                    pos++;
                    size_t endQuote = contentType.find('"', pos);
                    if (endQuote != std::string::npos) {
                        boundary = "--" + contentType.substr(pos, endQuote - pos);
                    }
                } else {
                    size_t endPos = contentType.find(';', pos);
                    if (endPos == std::string::npos) {
                        endPos = contentType.length();
                    }
                    boundary = "--" + contentType.substr(pos, endPos - pos);
                }
                
                boundary.erase(std::remove_if(boundary.begin(), boundary.end(), 
                    [](unsigned char c) { return std::isspace(c) || c == '"'; }), boundary.end());
            }
            
            if (boundary.empty()) {
                return;
            }
            
            std::string body = requestBody;
            std::string endBoundary = boundary + "--";
            
            size_t start = 0;
            while (true) {
                size_t partStart = body.find(boundary, start);
                if (partStart == std::string::npos) break;
                
                partStart += boundary.size();
                
                if (partStart + 2 <= body.size() && body.compare(partStart, 2, "--") == 0) break;
                
                if (partStart + 2 <= body.size() && body.compare(partStart, 2, "\r\n") == 0) partStart += 2;
                
                size_t partEnd = body.find(boundary, partStart);
                if (partEnd == std::string::npos) break;
                
                std::string part = body.substr(partStart, partEnd - partStart - 2);
                
                size_t headerEnd = part.find("\r\n\r\n");
                if (headerEnd == std::string::npos) { 
                    start = partEnd;
                    continue; 
                }
                
                std::string headerBlock = part.substr(0, headerEnd);
                std::string dataBlock = part.substr(headerEnd + 4);
                
                std::map<std::string, std::string> partHeaders;
                std::istringstream headerStream(headerBlock);
                std::string headerLine;
                while (std::getline(headerStream, headerLine)) {
                    if (!headerLine.empty() && headerLine.back() == '\r') headerLine.pop_back();
                    if (headerLine.empty()) continue;
                    
                    size_t colon = headerLine.find(":");
                    if (colon != std::string::npos) {
                        std::string key = headerLine.substr(0, colon);
                        std::string value = headerLine.substr(colon + 1);
                        while (!value.empty() && std::isspace(value.front())) value.erase(0, 1);
                        partHeaders[key] = value;
                    }
                }
                
                auto cdIt = partHeaders.find("Content-Disposition");
                if (cdIt != partHeaders.end()) {
                    std::string cd = cdIt->second;
                    
                    std::regex nameRe("name=\"([^\"]+)\"");
                    std::regex fnRe("filename=\"([^\"]*)\"");
                    std::smatch nameMatch, fnMatch;
                    std::string fieldName, fileName;
                    
                    if (std::regex_search(cd, nameMatch, nameRe)) fieldName = nameMatch[1];
                    if (std::regex_search(cd, fnMatch, fnRe)) fileName = fnMatch[1];
                    
                    if (!fileName.empty()) {
                        UploadedFile file;
                        file.fieldName = fieldName;
                        file.fileName = fileName;
                        file.contentType = partHeaders.count("Content-Type") ? partHeaders["Content-Type"] : "application/octet-stream";
                        file.size = dataBlock.size();
                        file.data.assign(dataBlock.begin(), dataBlock.end());
                        
                        uploadedFiles.push_back(std::move(file));
                    } else {
                        requestQueryParams[fieldName] = dataBlock;
                    }
                }
                
                start = partEnd;
            }
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
    return pimpl->requestBody;
}

nlohmann::json Request::json() const
{
    try
    {
        std::string contentType = header("Content-Type");
        bool isJsonContent = contentType.find("application/json") != std::string::npos ||
                             contentType.find("application/problem+json") != std::string::npos;

        if (isJsonContent || !pimpl->requestBody.empty())
        {
            return nlohmann::json::parse(pimpl->requestBody);
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

std::string Request::hostname() const
{
    auto it = pimpl->requestHeaders.find("Host");
    if (it != pimpl->requestHeaders.end()) {
        std::string host = it->second;
        size_t colonPos = host.find(':');
        if (colonPos != std::string::npos) {
            return host.substr(0, colonPos);
        }
        return host;
    }
    return "";
}

std::string Request::originalUrl() const
{
    if (!pimpl->fullUrl.empty()) {
        return pimpl->fullUrl;
    }

    std::string url = pimpl->requestPath;
    if (!pimpl->requestQueryString.empty()) {
        url += "?" + pimpl->requestQueryString;
    }
    return url;
}

std::string Request::protocol() const
{
    return pimpl->requestProtocol;
}

bool Request::secure() const
{
    return pimpl->isSecure;
}

std::string Request::cookie(const std::string& name) const
{
    auto it = pimpl->requestCookies.find(name);
    return (it != pimpl->requestCookies.end()) ? it->second : "";
}

std::map<std::string, std::string> Request::cookies() const
{
    return pimpl->requestCookies;
}

void Request::setRawRequest(const std::string& rawRequest)
{
    pimpl->rawRequest = rawRequest;
}

void Request::setRouteParam(const std::string& name, const std::string& value)
{
    pimpl->requestRouteParams[name] = value;
}

void Request::parse()
{
    std::vector<std::string> lines;
    std::istringstream iss(pimpl->rawRequest);
    std::string line;

    while (std::getline(iss, line))
    {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        lines.push_back(line);
    }

    if (lines.empty())
    {
        return;
    }

    pimpl->parseMethod(lines[0]);
    pimpl->parsePath(lines[0]);

    pimpl->parseHeaders(lines);
    pimpl->parseBody(lines);
}

void Request::setOriginalPath(const std::string& path)
{
    pimpl->originalRequestPath = path;
}

void Request::overridePath(const std::string& path)
{
    pimpl->requestPath = path;
}

std::vector<UploadedFile> Request::files() const {
    return pimpl->uploadedFiles;
}

void Request::setBody(const std::string& body)
{
    pimpl->requestBody = body;
    
    auto contentTypeIt = pimpl->requestHeaders.find("Content-Type");
    if (contentTypeIt != pimpl->requestHeaders.end() && 
        contentTypeIt->second.find("multipart/form-data") != std::string::npos) {
        
        std::string contentType = contentTypeIt->second;
        std::string boundary;
        
        std::string::size_type pos = contentType.find("boundary=");
        if (pos != std::string::npos) {
            pos += 9;
            
            if (pos < contentType.length() && contentType[pos] == '"') {
                pos++;
                size_t endQuote = contentType.find('"', pos);
                if (endQuote != std::string::npos) {
                    boundary = "--" + contentType.substr(pos, endQuote - pos);
                }
            } else {
                size_t endPos = contentType.find(';', pos);
                if (endPos == std::string::npos) {
                    endPos = contentType.length();
                }
                boundary = "--" + contentType.substr(pos, endPos - pos);
            }
            
            boundary.erase(std::remove_if(boundary.begin(), boundary.end(), 
                [](unsigned char c) { return std::isspace(c); }), boundary.end());
        }
        
        if (!boundary.empty() && !body.empty()) {
            size_t start = body.find(boundary);
            if (start != std::string::npos) {
                pimpl->uploadedFiles.clear();
                
                while (true) {
                    start = body.find(boundary, start);
                    if (start == std::string::npos) break;
                    
                    start += boundary.length();
                    
                    if (start + 2 <= body.size() && body.compare(start, 2, "--") == 0) {
                        break;
                    }
                    
                    if (start + 2 <= body.size() && body.compare(start, 2, "\r\n") == 0) {
                        start += 2;
                    }
                    
                    size_t nextBoundary = body.find(boundary, start);
                    if (nextBoundary == std::string::npos) break;
                    
                    size_t partEndWithoutCRLF = nextBoundary - 2;
                    if (partEndWithoutCRLF <= start) continue;
                    
                    std::string partContent = body.substr(start, partEndWithoutCRLF - start);
                    
                    size_t headerEnd = partContent.find("\r\n\r\n");
                    if (headerEnd == std::string::npos) {
                        start = nextBoundary;
                        continue;
                    }
                    
                    std::string headersPart = partContent.substr(0, headerEnd);
                    std::string bodyPart = partContent.substr(headerEnd + 4);
                    
                    std::map<std::string, std::string> headers;
                    std::istringstream headerStream(headersPart);
                    std::string line;
                    
                    while (std::getline(headerStream, line)) {
                        if (!line.empty() && line.back() == '\r') line.pop_back();
                        if (line.empty()) continue;
                        
                        size_t colonPos = line.find(':');
                        if (colonPos != std::string::npos) {
                            std::string name = line.substr(0, colonPos);
                            std::string value = line.substr(colonPos + 1);
                            value.erase(0, value.find_first_not_of(" \t"));
                            headers[name] = value;
                        }
                    }
                    
                    auto cdIt = headers.find("Content-Disposition");
                    if (cdIt != headers.end()) {
                        std::string cd = cdIt->second;
                        
                        std::string fieldName, fileName;
                        
                        std::regex nameRegex("name=\"([^\"]*)\"");
                        std::smatch nameMatch;
                        if (std::regex_search(cd, nameMatch, nameRegex) && nameMatch.size() > 1) {
                            fieldName = nameMatch[1];
                        }
                        
                        std::regex fileRegex("filename=\"([^\"]*)\"");
                        std::smatch fileMatch;
                        if (std::regex_search(cd, fileMatch, fileRegex) && fileMatch.size() > 1) {
                            fileName = fileMatch[1];
                        }
                        
                        if (!fileName.empty()) {
                            UploadedFile file;
                            file.fieldName = fieldName;
                            file.fileName = fileName;
                            file.contentType = headers.count("Content-Type") ? headers["Content-Type"] : "application/octet-stream";
                            file.size = bodyPart.size();
                            file.data.assign(bodyPart.begin(), bodyPart.end());
                            
                            pimpl->uploadedFiles.push_back(std::move(file));
                        } else if (!fieldName.empty()) {
                            pimpl->requestQueryParams[fieldName] = bodyPart;
                        }
                    }
                    
                    start = nextBoundary;
                }
            }
        }
    }
}

} // namespace boson
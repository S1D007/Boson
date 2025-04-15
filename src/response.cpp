#include "boson/response.hpp"
#include "../include/external/json.hpp"

#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace boson
{

class Response::Impl
{
  public:
    Impl() : statusCode(200), sentFlag(false) {}

    std::map<std::string, std::string> responseHeaders;
    std::string responseBody;
    int statusCode;
    bool sentFlag;

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
        case 500:
            return "Internal Server Error";
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

        for (const auto& header : responseHeaders)
        {
            ss << header.first << ": " << header.second << "\r\n";
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

} // namespace boson
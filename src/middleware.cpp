#include "boson/middleware.hpp"
#include "boson/request.hpp"
#include "boson/response.hpp"

#include <functional>
#include <memory>
#include <string>
#include <regex>

namespace boson
{

class NextFunction::Impl
{
  public:
    Impl() : hasNextFlag(false), error("") {}

    Middleware nextMiddleware;
    bool hasNextFlag;
    std::string error;
    const Request* request;
    Response* response;
};

NextFunction::NextFunction() : pimpl(std::make_unique<Impl>()) {}

NextFunction::~NextFunction() {}

void NextFunction::operator()()
{
    if (pimpl->hasNextFlag && pimpl->request && pimpl->response)
    {
        pimpl->nextMiddleware(*pimpl->request, *pimpl->response, *this);
    }
}

void NextFunction::operator()(const std::string& error)
{
    pimpl->error = error;
    (*this)();
}

void NextFunction::setNext(const Middleware& middleware)
{
    pimpl->nextMiddleware = middleware;
    pimpl->hasNextFlag = true;
}

bool NextFunction::hasNext() const
{
    return pimpl->hasNextFlag;
}

void NextFunction::setRequestResponse(const Request& req, Response& res)
{
    pimpl->request = &req;
    pimpl->response = &res;
}

MiddlewareChain::MiddlewareChain() {}

MiddlewareChain::~MiddlewareChain() {}

void MiddlewareChain::add(const Middleware& middleware)
{
    chain.emplace_back(middleware);
}

void MiddlewareChain::add(const std::string& path, const Middleware& middleware)
{
    chain.emplace_back(middleware, path);
}

bool MiddlewareChain::pathMatches(const std::string& pattern, const std::string& path)
{
    // Simple path matching
    if (pattern == path) {
        return true;
    }
    
    // Check for wildcard prefix match (e.g., "/api/*")
    if (pattern.length() > 0 && pattern.back() == '*') {
        std::string prefix = pattern.substr(0, pattern.length() - 1);
        return path.compare(0, prefix.length(), prefix) == 0;
    }
    
    // Regex-based path matching
    try {
        std::regex pathRegex(pattern);
        return std::regex_match(path, pathRegex);
    } catch (const std::regex_error& e) {
        return false;
    }
}

bool MiddlewareChain::execute(const Request& req, Response& res)
{
    if (chain.empty())
    {
        return true;
    }

    const std::string& requestPath = req.path();
    std::vector<Middleware> applicableMiddleware;
    
    for (const auto& entry : chain) {
        if (!entry.path.has_value() || pathMatches(entry.path.value(), requestPath)) {
            applicableMiddleware.push_back(entry.middleware);
        }
    }
    
    if (applicableMiddleware.empty()) {
        return true;
    }
    
    size_t index = 0;
    
    std::function<void(void)> runMiddleware = [&]() {
        if (index >= applicableMiddleware.size() || res.sent()) {
            return;
        }
        
        NextFunction next;
        next.setNext([&](const Request& r, Response& s, NextFunction& n) {
            index++;
            runMiddleware();
        });
        next.setRequestResponse(req, res);
        
        applicableMiddleware[index](req, res, next);
    };
    
    runMiddleware();
    
    return !res.sent();
}

} // namespace boson
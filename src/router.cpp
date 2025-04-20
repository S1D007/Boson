#include "boson/router.hpp"
#include "../include/external/json.hpp"
#include "boson/middleware.hpp"
#include "boson/request.hpp"
#include "boson/response.hpp"

#include <algorithm>
#include <map>
#include <regex>
#include <string>
#include <vector>

namespace boson
{

RouterPtr Router::create()
{
    return std::make_shared<Router>();
}

Router::Router() {}

Router::~Router() {}

Router& Router::get(const std::string& path, const RouteHandler& handler)
{
    return addRoute("GET", path, handler);
}

// Overload for middleware support
Router& Router::get(const std::string& path, const Middleware& middleware,
                    const RouteHandler& handler)
{
    std::vector<Middleware> middlewares = {middleware};
    return addRouteWithMiddleware("GET", path, handler, middlewares);
}

// Overload for multiple middleware
Router& Router::get(const std::string& path, const std::vector<Middleware>& middlewares,
                    const RouteHandler& handler)
{
    return addRouteWithMiddleware("GET", path, handler, middlewares);
}

Router& Router::post(const std::string& path, const RouteHandler& handler)
{
    return addRoute("POST", path, handler);
}

// Overload for middleware support
Router& Router::post(const std::string& path, const Middleware& middleware,
                     const RouteHandler& handler)
{
    std::vector<Middleware> middlewares = {middleware};
    return addRouteWithMiddleware("POST", path, handler, middlewares);
}

// Overload for multiple middleware
Router& Router::post(const std::string& path, const std::vector<Middleware>& middlewares,
                     const RouteHandler& handler)
{
    return addRouteWithMiddleware("POST", path, handler, middlewares);
}

Router& Router::put(const std::string& path, const RouteHandler& handler)
{
    return addRoute("PUT", path, handler);
}

Router& Router::put(const std::string& path, const Middleware& middleware,
                    const RouteHandler& handler)
{
    std::vector<Middleware> middlewares = {middleware};
    return addRouteWithMiddleware("PUT", path, handler, middlewares);
}

Router& Router::put(const std::string& path, const std::vector<Middleware>& middlewares,
                    const RouteHandler& handler)
{
    return addRouteWithMiddleware("PUT", path, handler, middlewares);
}

Router& Router::del(const std::string& path, const RouteHandler& handler)
{
    return addRoute("DELETE", path, handler);
}

Router& Router::del(const std::string& path, const Middleware& middleware,
                    const RouteHandler& handler)
{
    std::vector<Middleware> middlewares = {middleware};
    return addRouteWithMiddleware("DELETE", path, handler, middlewares);
}

Router& Router::del(const std::string& path, const std::vector<Middleware>& middlewares,
                    const RouteHandler& handler)
{
    return addRouteWithMiddleware("DELETE", path, handler, middlewares);
}

Router& Router::patch(const std::string& path, const RouteHandler& handler)
{
    return addRoute("PATCH", path, handler);
}

Router& Router::patch(const std::string& path, const Middleware& middleware,
                      const RouteHandler& handler)
{
    std::vector<Middleware> middlewares = {middleware};
    return addRouteWithMiddleware("PATCH", path, handler, middlewares);
}

Router& Router::patch(const std::string& path, const std::vector<Middleware>& middlewares,
                      const RouteHandler& handler)
{
    return addRouteWithMiddleware("PATCH", path, handler, middlewares);
}

Router& Router::use(const Middleware& middleware)
{
    routerMiddleware.push_back(middleware);
    return *this;
}

Router& Router::use(const std::string& path, const Router& router)
{
    subRouters.push_back(std::make_pair(path, router));
    return *this;
}

bool Router::handle(const Request& req, Response& res) const
{
    // First try to match against sub-routers
    for (const auto& pair : subRouters)
    {
        const std::string& basePath = pair.first;
        const Router& router = pair.second;

        // Check if the request path starts with the sub-router base path
        if (req.path().compare(0, basePath.length(), basePath) == 0)
        {
            // Adjust the path for the sub-router
            std::string adjustedPath = req.path().substr(basePath.length());
            if (adjustedPath.empty() || adjustedPath[0] != '/')
            {
                adjustedPath = "/" + adjustedPath;
            }

            // Create a modified copy of the request path
            std::string originalPath = req.path();

            // Temporarily change the request path (use const_cast since we'll restore it)
            Request& mutableReq = const_cast<Request&>(req);
            mutableReq.setOriginalPath(originalPath); // Store original path
            mutableReq.overridePath(adjustedPath);    // Override with adjusted path

            // Try to handle the request with the sub-router
            bool handled = router.handle(req, res);

            // Restore original path
            mutableReq.overridePath(originalPath);

            if (handled)
            {
                return true;
            }
        }
    }

    // If no sub-router matches, try to match routes directly
    for (const auto& route : routes)
    {
        std::map<std::string, std::string> params;
        if (route.method == req.method() && matchPath(route.path, req.path(), params))
        {
            // Set route parameters on the request
            Request& mutableReq = const_cast<Request&>(req);
            for (const auto& param : params)
            {
                mutableReq.setRouteParam(param.first, param.second);
            }

            bool continueProcessing = true;
            
            std::vector<Middleware> middlewareChain;
            

            std::copy(routerMiddleware.begin(), routerMiddleware.end(), 
                     std::back_inserter(middlewareChain));
            
            std::copy(route.middleware.begin(), route.middleware.end(), 
                     std::back_inserter(middlewareChain));
                     
            if (!middlewareChain.empty()) {
                size_t currentIndex = 0;
                
                std::function<void(const Request&, Response&)> executeNext;
                executeNext = [&](const Request& request, Response& response) {
                    if (currentIndex >= middlewareChain.size() || response.sent()) {
                        continueProcessing = true;
                        return;
                    }
                    
                    NextFunction next;
                    next.setNext([&](const Request& r, Response& s, NextFunction& n) {
                        currentIndex++;
                        executeNext(request, response);
                    });
                    next.setRequestResponse(request, response);
                    
                    continueProcessing = false;
                    middlewareChain[currentIndex](request, response, next);
                };
                
                executeNext(req, res);
                
                if (res.sent()) {
                    return true;
                }
            }

            if (continueProcessing && !res.sent()) {
                route.handler(req, res);
            }
            
            return true;
        }
    }

    // No matching route found
    return false;
}

Router& Router::addRoute(const std::string& method, const std::string& path,
                         const RouteHandler& handler)
{
    Route route;
    route.method = method;
    route.path = path;
    route.handler = handler;

    routes.push_back(route);
    return *this;
}

// Add support for route-specific middleware
Router& Router::addRouteWithMiddleware(const std::string& method, const std::string& path,
                                       const RouteHandler& handler,
                                       const std::vector<Middleware>& middleware)
{
    Route route;
    route.method = method;
    route.path = path;
    route.handler = handler;
    route.middleware = middleware;

    routes.push_back(route);
    return *this;
}

bool Router::matchPath(const std::string& pattern, const std::string& path,
                       std::map<std::string, std::string>& params) const
{
    if (pattern == path)
    {
        return true;
    }

    if (pattern.empty())
    {
        return false;
    }

    std::regex paramNameRegex(":([a-zA-Z0-9_]+)");
    std::smatch paramNameMatch;
    std::string patternCopy = pattern;
    std::vector<std::string> paramNames;

    while (std::regex_search(patternCopy, paramNameMatch, paramNameRegex))
    {
        paramNames.push_back(paramNameMatch[1].str());
        patternCopy = paramNameMatch.suffix();
    }

    std::string regexPattern = pattern;
    std::regex paramRegex(":[a-zA-Z0-9_]+");
    std::string regexReplacement = "([^/]+)";
    regexPattern = std::regex_replace(regexPattern, paramRegex, regexReplacement);

    regexPattern = "^" + regexPattern + "$";

    std::regex re(regexPattern);
    std::smatch match;

    if (std::regex_match(path, match, re))
    {

        for (size_t i = 0; i < paramNames.size(); i++)
        {
            if (i + 1 < match.size())
            {
                params[paramNames[i]] = match[i + 1].str();
            }
        }
        return true;
    }

    return false;
}

bool Router::matchPath(const std::string& pattern, const std::string& path) const
{
    std::map<std::string, std::string> params;
    return matchPath(pattern, path, params);
}

} // namespace boson
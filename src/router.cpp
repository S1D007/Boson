#include "boson/router.hpp"
#include "boson/request.hpp"
#include "boson/response.hpp"
#include "boson/middleware.hpp"
#include "../include/external/json.hpp"

#include <string>
#include <regex>
#include <vector>
#include <map>
#include <algorithm>

namespace boson {

RouterPtr Router::create() {
    return std::make_shared<Router>();
}

Router::Router() {
}

Router::~Router() {
}

Router& Router::get(const std::string& path, const RouteHandler& handler) {
    return addRoute("GET", path, handler);
}

Router& Router::post(const std::string& path, const RouteHandler& handler) {
    return addRoute("POST", path, handler);
}

Router& Router::put(const std::string& path, const RouteHandler& handler) {
    return addRoute("PUT", path, handler);
}

Router& Router::del(const std::string& path, const RouteHandler& handler) {
    return addRoute("DELETE", path, handler);
}

Router& Router::patch(const std::string& path, const RouteHandler& handler) {
    return addRoute("PATCH", path, handler);
}

Router& Router::use(const Middleware& middleware) {
    routerMiddleware.push_back(middleware);
    return *this;
}

Router& Router::use(const std::string& path, const Router& router) {
    subRouters.push_back(std::make_pair(path, router));
    return *this;
}

bool Router::handle(const Request& req, Response& res) const {
    
    for (const auto& pair : subRouters) {
        const std::string& basePath = pair.first;
        const Router& router = pair.second;
        
        
        if (req.path().compare(0, basePath.length(), basePath) == 0) {
            
            std::string adjustedPath = req.path().substr(basePath.length());
            if (adjustedPath.empty() || adjustedPath[0] != '/') {
                adjustedPath = "/" + adjustedPath;
            }
            
            
            
            
            
            if (router.handle(req, res)) {
                return true;
            }
        }
    }
    
    
    for (const auto& route : routes) {
        std::map<std::string, std::string> params;
        if (route.method == req.method() && matchPath(route.path, req.path(), params)) {
            
            bool continueProcessing = true;
            
            
            Request& mutableReq = const_cast<Request&>(req);
            for (const auto& param : params) {
                mutableReq.setRouteParam(param.first, param.second);
            }
            
            for (const auto& middleware : routerMiddleware) {
                NextFunction next;
                next.setNext([&continueProcessing](const Request&, Response&, NextFunction&) {
                    continueProcessing = true;
                });
                
                middleware(req, res, next);
                
                if (!continueProcessing) {
                    return true;
                }
                
                if (res.sent()) {
                    return true;
                }
            }
            
            
            for (const auto& middleware : route.middleware) {
                NextFunction next;
                next.setNext([&continueProcessing](const Request&, Response&, NextFunction&) {
                    continueProcessing = true;
                });
                
                middleware(req, res, next);
                
                if (!continueProcessing) {
                    return true;
                }
                
                if (res.sent()) {
                    return true;
                }
            }
            
            
            route.handler(req, res);
            return true;
        }
    }
    
    
    return false;
}

Router& Router::addRoute(const std::string& method, const std::string& path, const RouteHandler& handler) {
    Route route;
    route.method = method;
    route.path = path;
    route.handler = handler;
    
    routes.push_back(route);
    return *this;
}

bool Router::matchPath(const std::string& pattern, const std::string& path, std::map<std::string, std::string>& params) const {
    if (pattern == path) {
        return true;
    }
    
    if (pattern.empty()) {
        return false;
    }
    
    
    std::regex paramNameRegex(":([a-zA-Z0-9_]+)");
    std::smatch paramNameMatch;
    std::string patternCopy = pattern;
    std::vector<std::string> paramNames;
    
    while (std::regex_search(patternCopy, paramNameMatch, paramNameRegex)) {
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
    
    if (std::regex_match(path, match, re)) {
        
        for (size_t i = 0; i < paramNames.size(); i++) {
            if (i + 1 < match.size()) {
                params[paramNames[i]] = match[i + 1].str();
            }
        }
        return true;
    }
    
    return false;
}

bool Router::matchPath(const std::string& pattern, const std::string& path) const {
    std::map<std::string, std::string> params;
    return matchPath(pattern, path, params);
}

} 
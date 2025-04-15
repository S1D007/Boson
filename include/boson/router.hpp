#ifndef BOSON_ROUTER_HPP
#define BOSON_ROUTER_HPP

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <memory>
#include "request.hpp"
#include "response.hpp"
#include "middleware.hpp"

namespace boson {

class Router;
using RouterPtr = std::shared_ptr<Router>;

/**
 * @brief Route handler function type
 * @param req The HTTP request
 * @param res The HTTP response
 * @return void
 */
using RouteHandler = std::function<void(const Request&, Response&)>;

/**
 * @class Router
 * @brief Router class for handling HTTP routes
 */
class Router {
public:
    Router();
    ~Router();

    /**
     * @brief Register a GET route handler
     * @param path The route path
     * @param handler The handler function
     * @return Reference to this router for method chaining
     */
    Router& get(const std::string& path, const RouteHandler& handler);
    
    /**
     * @brief Register a POST route handler
     * @param path The route path
     * @param handler The handler function
     * @return Reference to this router for method chaining
     */
    Router& post(const std::string& path, const RouteHandler& handler);
    
    /**
     * @brief Register a PUT route handler
     * @param path The route path
     * @param handler The handler function
     * @return Reference to this router for method chaining
     */
    Router& put(const std::string& path, const RouteHandler& handler);
    
    /**
     * @brief Register a DELETE route handler
     * @param path The route path
     * @param handler The handler function
     * @return Reference to this router for method chaining
     */
    Router& del(const std::string& path, const RouteHandler& handler);
    
    /**
     * @brief Register a PATCH route handler
     * @param path The route path
     * @param handler The handler function
     * @return Reference to this router for method chaining
     */
    Router& patch(const std::string& path, const RouteHandler& handler);
    
    /**
     * @brief Add middleware to the router
     * @param middleware The middleware function to add
     * @return Reference to this router for method chaining
     */
    Router& use(const Middleware& middleware);
    
    /**
     * @brief Mount a sub-router at the specified path
     * @param path The base path for the router
     * @param router The router to mount
     * @return Reference to this router for method chaining
     */
    Router& use(const std::string& path, const Router& router);
    
    /**
     * @brief Handle a request
     * @param req The HTTP request
     * @param res The HTTP response
     * @return True if the request was handled, false otherwise
     */
    bool handle(const Request& req, Response& res) const;

    /**
     * @brief Create a new router
     * @return A new router instance
     */
    static RouterPtr create();

private:
    struct Route {
        std::string method;
        std::string path;
        RouteHandler handler;
        std::vector<Middleware> middleware;
    };
    
    std::vector<Route> routes;
    std::vector<Middleware> routerMiddleware;
    std::vector<std::pair<std::string, Router>> subRouters;
    
    /**
     * @brief Add a route to the router
     * @param method The HTTP method
     * @param path The route path
     * @param handler The handler function
     * @return Reference to this router for method chaining
     */
    Router& addRoute(const std::string& method, const std::string& path, const RouteHandler& handler);
    
    /**
     * @brief Match a path against a route pattern
     * @param pattern The route pattern
     * @param path The request path
     * @return True if the path matches the pattern, false otherwise
     */
    bool matchPath(const std::string& pattern, const std::string& path) const;
    
    /**
     * @brief Match a path against a route pattern and extract parameters
     * @param pattern The route pattern
     * @param path The request path
     * @param params Map to store extracted parameters
     * @return True if the path matches the pattern, false otherwise
     */
    bool matchPath(const std::string& pattern, const std::string& path, std::map<std::string, std::string>& params) const;
};

} 

#endif 
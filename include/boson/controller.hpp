#ifndef BOSON_CONTROLLER_HPP
#define BOSON_CONTROLLER_HPP

#include <string>
#include <map>
#include <functional>
#include <memory>
#include <vector>
#include <type_traits>
#include "router.hpp"
#include "request.hpp"
#include "response.hpp"

namespace boson {

/**
 * @brief HTTP method types
 */
enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    OPTIONS,
    HEAD
};

/**
 * @brief Route decorator information
 */
struct RouteInfo {
    HttpMethod method;
    std::string path;
    std::string handlerName;
    RouteHandler handlerFunction;
};

/**
 * @brief Base class for controllers
 */
class Controller {
public:
    Controller();
    virtual ~Controller();
    
    /**
     * @brief Get the base path for this controller
     * @return The base path
     */
    virtual std::string basePath() const = 0;
    
    /**
     * @brief Register routes with a router
     * @param router The router to register routes with
     */
    void registerRoutes(Router& router);

protected:
    /**
     * @brief Register a route with a handler function
     * @param method The HTTP method
     * @param path The route path
     * @param handlerName The name of the handler method (for debugging)
     * @param handler The handler function
     */
    void registerRoute(HttpMethod method, const std::string& path, const std::string& handlerName, const RouteHandler& handler);
    
    std::vector<RouteInfo> routes;
};

/**
 * @class ControllerFactory
 * @brief Factory for creating controllers
 */
class ControllerFactory {
public:
    /**
     * @brief Register a controller type
     * @tparam T The controller type
     */
    template<typename T>
    static void registerController() {
        static_assert(std::is_base_of<Controller, T>::value, "Type must inherit from Controller");
        
    }
    
    /**
     * @brief Create a controller instance
     * @tparam T The controller type
     * @return A shared pointer to the controller
     */
    template<typename T>
    static std::shared_ptr<T> createController() {
        static_assert(std::is_base_of<Controller, T>::value, "Type must inherit from Controller");
        return std::make_shared<T>();
    }
};

} 

#endif 
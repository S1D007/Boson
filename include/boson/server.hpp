#ifndef BOSON_SERVER_HPP
#define BOSON_SERVER_HPP

#include <string>
#include <memory>
#include <functional>
#include <vector>
#include "router.hpp"
#include "middleware.hpp"
#include "request.hpp"
#include "response.hpp"

namespace boson {


using ErrorHandler = std::function<void(const std::exception&, const Request&, Response&)>;

/**
 * @class Server
 * @brief Main server class for the Boson framework
 */
class Server {
public:
    Server();
    ~Server();
    
    /**
     * @brief Configure the server
     * @param port The port to listen on
     * @param host The host to bind to
     * @return Reference to this server for method chaining
     */
    Server& configure(int port = 3000, const std::string& host = "127.0.0.1");
    
    /**
     * @brief Start the server and listen for requests
     * @return Non-zero on error
     */
    int listen();
    
    /**
     * @brief Stop the server
     */
    void stop();
    
    /**
     * @brief Add global middleware to the server
     * @param middleware The middleware function to add
     * @return Reference to this server for method chaining
     */
    Server& use(const Middleware& middleware);
    
    /**
     * @brief Mount a router at the specified path
     * @param path The base path for the router
     * @param router The router to mount
     * @return Reference to this server for method chaining
     */
    Server& use(const std::string& path, const Router& router);
    
    /**
     * @brief Register a GET route handler
     * @param path The route path
     * @param handler The handler function
     * @return Reference to this server for method chaining
     */
    Server& get(const std::string& path, const RouteHandler& handler);
    
    /**
     * @brief Register a POST route handler
     * @param path The route path
     * @param handler The handler function
     * @return Reference to this server for method chaining
     */
    Server& post(const std::string& path, const RouteHandler& handler);
    
    /**
     * @brief Register a PUT route handler
     * @param path The route path
     * @param handler The handler function
     * @return Reference to this server for method chaining
     */
    Server& put(const std::string& path, const RouteHandler& handler);
    
    /**
     * @brief Register a DELETE route handler
     * @param path The route path
     * @param handler The handler function
     * @return Reference to this server for method chaining
     */
    Server& del(const std::string& path, const RouteHandler& handler);
    
    /**
     * @brief Register a PATCH route handler
     * @param path The route path
     * @param handler The handler function
     * @return Reference to this server for method chaining
     */
    Server& patch(const std::string& path, const RouteHandler& handler);
    
    /**
     * @brief Set the error handler for the server
     * @param handler The error handler function
     * @return Reference to this server for method chaining
     */
    Server& setErrorHandler(const ErrorHandler& handler);

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
    Router mainRouter;
    std::vector<Middleware> globalMiddleware;
    int port;
    std::string host;
};

} 

#endif 
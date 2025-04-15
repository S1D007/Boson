#ifndef BOSON_ROUTE_BINDER_HPP
#define BOSON_ROUTE_BINDER_HPP

#include "controller.hpp"
#include "router.hpp"
#include "server.hpp"
#include <functional>
#include <type_traits>

namespace boson
{
class Server;

/**
 * @class RouteBinder
 * @brief Router extension with method binding support for controller methods
 */
template <typename ControllerT> class RouteBinder
{
  public:
    explicit RouteBinder(std::shared_ptr<ControllerT> controller, const std::string& basePath = "")
        : controller_(controller), basePath_(basePath)
    {
        static_assert(std::is_base_of<Controller, ControllerT>::value,
                      "Controller type must inherit from boson::Controller");
        router_ = Router::create();
    }

    /**
     * @brief Register a GET route handler using a controller method
     */
    template <typename F> RouteBinder& get(const std::string& path, F handler)
    {
        registerHandler("GET", path, handler);
        return *this;
    }

    /**
     * @brief Register a GET route handler with middleware using a controller method
     */
    template <typename F>
    RouteBinder& get(const std::string& path, const Middleware& middleware, F handler)
    {
        registerHandlerWithMiddleware("GET", path, {middleware}, handler);
        return *this;
    }

    /**
     * @brief Register a GET route handler with multiple middleware using a controller method
     */
    template <typename F>
    RouteBinder& get(const std::string& path, const std::vector<Middleware>& middlewares, F handler)
    {
        registerHandlerWithMiddleware("GET", path, middlewares, handler);
        return *this;
    }

    /**
     * @brief Register a POST route handler using a controller method
     */
    template <typename F> RouteBinder& post(const std::string& path, F handler)
    {
        registerHandler("POST", path, handler);
        return *this;
    }

    /**
     * @brief Register a POST route handler with middleware using a controller method
     */
    template <typename F>
    RouteBinder& post(const std::string& path, const Middleware& middleware, F handler)
    {
        registerHandlerWithMiddleware("POST", path, {middleware}, handler);
        return *this;
    }

    /**
     * @brief Register a POST route handler with multiple middleware using a controller method
     */
    template <typename F>
    RouteBinder& post(const std::string& path, const std::vector<Middleware>& middlewares,
                      F handler)
    {
        registerHandlerWithMiddleware("POST", path, middlewares, handler);
        return *this;
    }

    /**
     * @brief Register a PUT route handler using a controller method
     */
    template <typename F> RouteBinder& put(const std::string& path, F handler)
    {
        registerHandler("PUT", path, handler);
        return *this;
    }

    /**
     * @brief Register a PUT route handler with middleware using a controller method
     */
    template <typename F>
    RouteBinder& put(const std::string& path, const Middleware& middleware, F handler)
    {
        registerHandlerWithMiddleware("PUT", path, {middleware}, handler);
        return *this;
    }

    /**
     * @brief Register a PUT route handler with multiple middleware using a controller method
     */
    template <typename F>
    RouteBinder& put(const std::string& path, const std::vector<Middleware>& middlewares, F handler)
    {
        registerHandlerWithMiddleware("PUT", path, middlewares, handler);
        return *this;
    }

    /**
     * @brief Register a DELETE route handler using a controller method
     */
    template <typename F> RouteBinder& del(const std::string& path, F handler)
    {
        registerHandler("DELETE", path, handler);
        return *this;
    }

    /**
     * @brief Register a DELETE route handler with middleware using a controller method
     */
    template <typename F>
    RouteBinder& del(const std::string& path, const Middleware& middleware, F handler)
    {
        registerHandlerWithMiddleware("DELETE", path, {middleware}, handler);
        return *this;
    }

    /**
     * @brief Register a DELETE route handler with multiple middleware using a controller method
     */
    template <typename F>
    RouteBinder& del(const std::string& path, const std::vector<Middleware>& middlewares, F handler)
    {
        registerHandlerWithMiddleware("DELETE", path, middlewares, handler);
        return *this;
    }

    /**
     * @brief Register a PATCH route handler using a controller method
     */
    template <typename F> RouteBinder& patch(const std::string& path, F handler)
    {
        registerHandler("PATCH", path, handler);
        return *this;
    }

    /**
     * @brief Register a PATCH route handler with middleware using a controller method
     */
    template <typename F>
    RouteBinder& patch(const std::string& path, const Middleware& middleware, F handler)
    {
        registerHandlerWithMiddleware("PATCH", path, {middleware}, handler);
        return *this;
    }

    /**
     * @brief Register a PATCH route handler with multiple middleware using a controller method
     */
    template <typename F>
    RouteBinder& patch(const std::string& path, const std::vector<Middleware>& middlewares,
                       F handler)
    {
        registerHandlerWithMiddleware("PATCH", path, middlewares, handler);
        return *this;
    }

    /**
     * @brief Mount this router on another router
     */
    void mountOn(Router& parentRouter)
    {
        std::string mountPath = controller_->basePath();
        if (!basePath_.empty())
        {
            if (mountPath.empty())
            {
                mountPath = basePath_;
            }
            else
            {
                mountPath = basePath_ + (mountPath[0] == '/' ? "" : "/") + mountPath;
            }
        }
        parentRouter.use(mountPath, *router_);
    }

    /**
     * @brief Mount this router on a server (overload for Server)
     *
     * This overload is specifically for Server objects
     */
    void mountOn(Server* server)
    {
        std::string mountPath = controller_->basePath();
        if (!basePath_.empty())
        {
            if (mountPath.empty())
            {
                mountPath = basePath_;
            }
            else
            {
                mountPath = basePath_ + (mountPath[0] == '/' ? "" : "/") + mountPath;
            }
        }
        server->use(mountPath, *router_);
    }

    /**
     * @brief Get the underlying router
     */
    RouterPtr getRouter() { return router_; }

  private:
    std::shared_ptr<ControllerT> controller_;
    RouterPtr router_;
    std::string basePath_;

    /**
     * @brief Register a route handler using a controller method
     */
    template <typename F>
    void registerHandler(const std::string& method, const std::string& path, F handler)
    {
        auto routeHandler = [this, handler](const Request& req, Response& res)
        { (controller_.get()->*handler)(req, res); };

        if (method == "GET")
        {
            router_->get(path, routeHandler);
        }
        else if (method == "POST")
        {
            router_->post(path, routeHandler);
        }
        else if (method == "PUT")
        {
            router_->put(path, routeHandler);
        }
        else if (method == "DELETE")
        {
            router_->del(path, routeHandler);
        }
        else if (method == "PATCH")
        {
            router_->patch(path, routeHandler);
        }
    }

    /**
     * @brief Register a route handler with middleware using a controller method
     */
    template <typename F>
    void registerHandlerWithMiddleware(const std::string& method, const std::string& path,
                                       const std::vector<Middleware>& middlewares, F handler)
    {
        auto routeHandler = [this, handler](const Request& req, Response& res)
        { (controller_.get()->*handler)(req, res); };

        if (method == "GET")
        {
            router_->get(path, middlewares, routeHandler);
        }
        else if (method == "POST")
        {
            router_->post(path, middlewares, routeHandler);
        }
        else if (method == "PUT")
        {
            router_->put(path, middlewares, routeHandler);
        }
        else if (method == "DELETE")
        {
            router_->del(path, middlewares, routeHandler);
        }
        else if (method == "PATCH")
        {
            router_->patch(path, middlewares, routeHandler);
        }
    }
};

/**
 * @brief Create a new route binder for a controller
 */
template <typename ControllerT>
RouteBinder<ControllerT> createRouter(std::shared_ptr<ControllerT> controller,
                                      const std::string& basePath = "")
{
    return RouteBinder<ControllerT>(controller, basePath);
}

} // namespace boson

#endif // BOSON_ROUTE_BINDER_HPP
#include "boson/controller.hpp"
#include "boson/request.hpp"
#include "boson/response.hpp"
#include "boson/router.hpp"

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace boson
{

Controller::Controller() {}

Controller::~Controller() {}

void Controller::registerRoutes(Router& router)
{
    std::string base = basePath();

    for (const auto& routeInfo : routes)
    {
        std::string path = base;
        if (path.back() != '/' && !routeInfo.path.empty() && routeInfo.path[0] != '/')
        {
            path += "/";
        }
        path += routeInfo.path;

        size_t pos = 0;
        while ((pos = path.find("//", pos)) != std::string::npos)
        {
            path.erase(pos, 1);
        }

        switch (routeInfo.method)
        {
        case HttpMethod::GET:
            router.get(path, routeInfo.handlerFunction);
            break;
        case HttpMethod::POST:
            router.post(path, routeInfo.handlerFunction);
            break;
        case HttpMethod::PUT:
            router.put(path, routeInfo.handlerFunction);
            break;
        case HttpMethod::DELETE:
            router.del(path, routeInfo.handlerFunction);
            break;
        case HttpMethod::PATCH:
            router.patch(path, routeInfo.handlerFunction);
            break;
        default:
            break;
        }
    }
}

void Controller::registerRoute(HttpMethod method, const std::string& path,
                               const std::string& handlerName, const RouteHandler& handler)
{
    RouteInfo info;
    info.method = method;
    info.path = path;
    info.handlerName = handlerName;
    info.handlerFunction = handler;

    routes.push_back(info);
}

static std::map<std::string, std::function<std::shared_ptr<Controller>()>> controllerRegistry;

} // namespace boson
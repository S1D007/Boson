#include "boson/middleware.hpp"
#include "boson/request.hpp"
#include "boson/response.hpp"

#include <functional>
#include <memory>
#include <string>

namespace boson
{

class NextFunction::Impl
{
  public:
    Impl() : hasNextFlag(false), error("") {}

    Middleware nextMiddleware;
    bool hasNextFlag;
    std::string error;
};

NextFunction::NextFunction() : pimpl(std::make_unique<Impl>()) {}

NextFunction::~NextFunction() {}

void NextFunction::operator()()
{
    if (pimpl->hasNextFlag)
    {

        Request req;
        Response res;
        pimpl->nextMiddleware(req, res, *this);
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

MiddlewareChain::MiddlewareChain() {}

MiddlewareChain::~MiddlewareChain() {}

void MiddlewareChain::add(const Middleware& middleware)
{
    chain.push_back(middleware);
}

bool MiddlewareChain::execute(const Request& req, Response& res)
{
    if (chain.empty())
    {
        return true;
    }

    size_t index = 0;

    std::function<void(const std::string&)> nextFunction = [&](const std::string& error)
    {
        if (!error.empty())
        {

            return;
        }

        index++;

        if (index >= chain.size())
        {
            return;
        }

        NextFunction next;
        next.setNext([&](const Request& r, Response& s, NextFunction& n) { nextFunction(""); });

        chain[index](req, res, next);
    };

    NextFunction next;
    next.setNext([&](const Request& r, Response& s, NextFunction& n) { nextFunction(""); });

    chain[0](req, res, next);

    return !res.sent();
}

} // namespace boson
#ifndef BOSON_HPP
#define BOSON_HPP

#include "controller.hpp"
#include "error_handler.hpp"
#include "middleware.hpp"
#include "request.hpp"
#include "response.hpp"
#include "route_binder.hpp"
#include "router.hpp"
#include "server.hpp"
#include "static_files.hpp"

namespace boson
{

/**
 * @brief Initialize the Boson framework
 *
 * This function should be called before using any Boson functionality.
 * It initializes the framework and registers built-in components.
 */
inline void initialize() {}

} // namespace boson

#endif
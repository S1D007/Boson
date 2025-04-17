#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using socket_t = SOCKET;
#define SOCKET_ERROR_VALUE INVALID_SOCKET
#ifndef EAGAIN
#define EAGAIN WSAEWOULDBLOCK
#endif
#ifndef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif
inline int close_socket(SOCKET socket) { return closesocket(socket); }
#else
#ifdef __APPLE__
#include <sys/event.h>
#endif
#if defined(__linux__) || (!defined(__APPLE__) && !defined(_WIN32))
#include <sys/epoll.h>
#endif
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
using socket_t = int;
#define SOCKET_ERROR_VALUE (-1)
inline int close_socket(int fd) { return ::close(fd); }
#endif

#include "boson/server.hpp"
#include "boson/error_handler.hpp"
#include "boson/middleware.hpp"
#include "boson/request.hpp"
#include "boson/response.hpp"
#include "boson/router.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace boson
{

// Forward declarations
class Connection;
class EventLoop;

struct Buffer
{
    char* data;
    size_t capacity;
    size_t size;

    Buffer(size_t initialCapacity = 8192) : capacity(initialCapacity), size(0)
    {
        data = new char[capacity];
    }

    ~Buffer() { delete[] data; }

    void ensureSpace(size_t requiredSpace)
    {
        if (size + requiredSpace > capacity)
        {
            size_t newCapacity = std::max(capacity * 2, size + requiredSpace);
            char* newData = new char[newCapacity];
            memcpy(newData, data, size);
            delete[] data;
            data = newData;
            capacity = newCapacity;
        }
    }

    void append(const char* src, size_t len)
    {
        ensureSpace(len);
        memcpy(data + size, src, len);
        size += len;
    }

    void clear() { size = 0; }
};

class Connection
{
  public:
    Connection(socket_t socket, EventLoop* eventLoop, Router& router,
               const std::vector<Middleware>& middleware, const ErrorHandler& errorHandler)
        : socket_(socket), eventLoop_(eventLoop), router_(router), middleware_(middleware),
          errorHandler_(errorHandler), readBuffer_(16384), writeBuffer_(16384), keepAlive_(true),
          isWriting_(false), state_(State::READ_REQUEST)
    {
    }

    ~Connection() { closeConnection(); }

    void closeConnection()
    {
        if (socket_ != SOCKET_ERROR_VALUE)
        {
            close_socket(socket_);
            socket_ = SOCKET_ERROR_VALUE;
        }
    }

    bool onReadable();
    bool onWritable();
    void processRequest();

    socket_t socket() const { return socket_; }
    bool isWriting() const { return isWriting_; }

  private:
    enum class State
    {
        READ_REQUEST,
        PROCESS_REQUEST,
        WRITE_RESPONSE,
        FINISHED
    };

    socket_t socket_;
    EventLoop* eventLoop_;
    Router& router_;
    const std::vector<Middleware>& middleware_;
    const ErrorHandler& errorHandler_;
    Buffer readBuffer_;
    Buffer writeBuffer_;
    bool keepAlive_;
    bool isWriting_;
    State state_;
};

class EventLoop
{
  public:
    EventLoop() : running_(false) {}

    bool initialize()
    {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            std::cerr << "Failed to initialize Winsock" << std::endl;
            return false;
        }
#elif defined(__APPLE__) || defined(__MACH__)
        kqueueFd_ = kqueue();
        if (kqueueFd_ < 0)
        {
            std::cerr << "Failed to create kqueue instance" << std::endl;
            return false;
        }
#else
        epollFd_ = epoll_create1(0);
        if (epollFd_ < 0)
        {
            std::cerr << "Failed to create epoll instance" << std::endl;
            return false;
        }
#endif
        return true;
    }

    void cleanup()
    {
#ifdef _WIN32
        WSACleanup();
#elif defined(__APPLE__) || defined(__MACH__)
        if (kqueueFd_ >= 0)
        {
            ::close(kqueueFd_);
            kqueueFd_ = -1;
        }
#else
        if (epollFd_ >= 0)
        {
            ::close(epollFd_);
            epollFd_ = -1;
        }
#endif
    }

    bool addSocket(socket_t socket, bool isListening = false)
    {
#ifdef _WIN32
        u_long mode = 1;
        ioctlsocket(socket, FIONBIO, &mode);
#else
        int flags = fcntl(socket, F_GETFL, 0);
        fcntl(socket, F_SETFL, flags | O_NONBLOCK);

#if defined(__APPLE__) || defined(__MACH__)
        struct kevent event[2];
        EV_SET(&event[0], socket, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, nullptr);
        EV_SET(&event[1], socket, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, nullptr);
        if (kevent(kqueueFd_, event, 2, nullptr, 0, nullptr) < 0)
        {
            std::cerr << "Failed to add socket to kqueue" << std::endl;
            return false;
        }
#else
        struct epoll_event event;
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = socket;
        if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, socket, &event) < 0)
        {
            std::cerr << "Failed to add socket to epoll" << std::endl;
            return false;
        }
#endif
#endif
        if (isListening)
        {
            listenerSocket_ = socket;
        }
        else
        {
            auto connection = std::make_shared<Connection>(socket, this, router_, globalMiddleware_,
                                                           errorHandler_);
            connections_[socket] = connection;
        }
        return true;
    }

    void updateEvents(socket_t socket, bool readable, bool writable)
    {
#ifdef _WIN32
#elif defined(__APPLE__) || defined(__MACH__)
        struct kevent event[2];
        int n = 0;

        EV_SET(&event[n++], socket, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, nullptr);

        if (writable)
        {
            EV_SET(&event[n++], socket, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, nullptr);
        }
        else
        {
            EV_SET(&event[n++], socket, EVFILT_WRITE, EV_DISABLE, 0, 0, nullptr);
        }

        kevent(kqueueFd_, event, n, nullptr, 0, nullptr);
#else
        struct epoll_event event;
        event.data.fd = socket;
        event.events = EPOLLET;
        if (readable)
            event.events |= EPOLLIN;
        if (writable)
            event.events |= EPOLLOUT;
        epoll_ctl(epollFd_, EPOLL_CTL_MOD, socket, &event);
#endif
    }

    void removeConnection(socket_t socket)
    {
#ifdef _WIN32
#elif defined(__APPLE__) || defined(__MACH__)
        struct kevent event[2];
        EV_SET(&event[0], socket, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
        EV_SET(&event[1], socket, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
        kevent(kqueueFd_, event, 2, nullptr, 0, nullptr);
#else
        epoll_ctl(epollFd_, EPOLL_CTL_DEL, socket, nullptr);
#endif
        connections_.erase(socket);
    }

    void run()
    {
        running_ = true;

        const int MAX_EVENTS = 64;
#if defined(__APPLE__) || defined(__MACH__)
        struct kevent events[MAX_EVENTS];
#elif !defined(_WIN32)
        struct epoll_event events[MAX_EVENTS];
#endif

        while (running_)
        {
#ifdef _WIN32
            fd_set readFds, writeFds, errorFds;
            FD_ZERO(&readFds);
            FD_ZERO(&writeFds);
            FD_ZERO(&errorFds);

            FD_SET(listenerSocket_, &readFds);
            FD_SET(listenerSocket_, &errorFds);

            for (const auto& pair : connections_)
            {
                FD_SET(pair.first, &readFds);
                FD_SET(pair.first, &errorFds);
                if (pair.second->isWriting())
                {
                    FD_SET(pair.first, &writeFds);
                }
            }

            struct timeval tv = {0, 100000}; // 100ms timeout
            int res = select(0, &readFds, &writeFds, &errorFds, &tv);

            if (res > 0)
            {
                if (FD_ISSET(listenerSocket_, &readFds))
                {
                    acceptNewConnections();
                }

                std::vector<socket_t> toRemove;
                for (const auto& pair : connections_)
                {
                    socket_t socket = pair.first;
                    if (FD_ISSET(socket, &readFds))
                    {
                        if (!pair.second->onReadable())
                        {
                            toRemove.push_back(socket);
                            continue;
                        }
                    }

                    if (FD_ISSET(socket, &writeFds))
                    {
                        if (!pair.second->onWritable())
                        {
                            toRemove.push_back(socket);
                            continue;
                        }
                    }

                    if (FD_ISSET(socket, &errorFds))
                    {
                        toRemove.push_back(socket);
                    }
                }

                for (socket_t socket : toRemove)
                {
                    removeConnection(socket);
                }
            }
#elif defined(__APPLE__) || defined(__MACH__)
            // kqueue-based implementation for macOS
            struct timespec timeout = {0, 100000000}; // 100ms timeout
            int nEvents = kevent(kqueueFd_, nullptr, 0, events, MAX_EVENTS, &timeout);

            if (nEvents < 0)
            {
                if (errno == EINTR)
                    continue;
                std::cerr << "kevent error: " << strerror(errno) << std::endl;
                break;
            }

            for (int i = 0; i < nEvents; ++i)
            {
                socket_t socket = static_cast<socket_t>(events[i].ident);

                if (socket == listenerSocket_)
                {
                    if (events[i].filter == EVFILT_READ)
                    {
                        acceptNewConnections();
                    }
                    continue;
                }

                auto it = connections_.find(socket);
                if (it == connections_.end())
                    continue;

                bool error = false;

                if (events[i].flags & (EV_EOF | EV_ERROR))
                {
                    error = true;
                }
                else
                {
                    if (events[i].filter == EVFILT_READ)
                    {
                        if (!it->second->onReadable())
                        {
                            error = true;
                        }
                    }

                    if (events[i].filter == EVFILT_WRITE)
                    {
                        if (!it->second->onWritable())
                        {
                            error = true;
                        }
                    }
                }

                if (error)
                {
                    removeConnection(socket);
                }
            }
#else
            // Epoll-based implementation for Linux
            int nfds = epoll_wait(epollFd_, events, MAX_EVENTS, 100); // 100ms timeout

            for (int i = 0; i < nfds; ++i)
            {
                socket_t socket = events[i].data.fd;

                if (socket == listenerSocket_)
                {
                    acceptNewConnections();
                    continue;
                }

                auto it = connections_.find(socket);
                if (it == connections_.end())
                    continue;

                bool error = false;

                if (events[i].events & EPOLLIN)
                {
                    if (!it->second->onReadable())
                    {
                        error = true;
                    }
                }

                if (events[i].events & EPOLLOUT)
                {
                    if (!it->second->onWritable())
                    {
                        error = true;
                    }
                }

                if (events[i].events & (EPOLLERR | EPOLLHUP))
                {
                    error = true;
                }

                if (error)
                {
                    removeConnection(socket);
                }
            }
#endif

            auto now = std::chrono::steady_clock::now();
            if (now - lastCleanupTime_ > std::chrono::seconds(60))
            {
                cleanupIdleConnections();
                lastCleanupTime_ = now;
            }
        }
    }

    void stop()
    {
        running_ = false;

        std::vector<socket_t> socketsToRemove;
        for (const auto& pair : connections_)
        {
            socketsToRemove.push_back(pair.first);
        }

        for (socket_t socket : socketsToRemove)
        {
            removeConnection(socket);
        }

        connections_.clear();

        if (listenerSocket_ != SOCKET_ERROR_VALUE)
        {
            close_socket(listenerSocket_);
            listenerSocket_ = SOCKET_ERROR_VALUE;
        }

        cleanup();
    }

    void setRouter(const Router& router) { router_ = router; }

    void setGlobalMiddleware(const std::vector<Middleware>& middleware)
    {
        globalMiddleware_ = middleware;
    }

    void setErrorHandler(const ErrorHandler& handler) { errorHandler_ = handler; }

  private:
    void acceptNewConnections()
    {
        while (true)
        {
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);

            socket_t clientSocket =
                accept(listenerSocket_, (struct sockaddr*)&clientAddr, &clientAddrLen);

            if (clientSocket == SOCKET_ERROR_VALUE)
            {
#ifdef _WIN32
                int error = WSAGetLastError();
                if (error == WSAEWOULDBLOCK || error == WSAECONNRESET)
                {
                    break; // No more connections to accept
                }
#else
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    break; // No more connections to accept
                }
#endif
                std::cerr << "Failed to accept connection" << std::endl;
                break;
            }

            // Add client socket to event loop
            addSocket(clientSocket);
        }
    }

    void cleanupIdleConnections()
    {
        auto now = std::chrono::steady_clock::now();
        std::vector<socket_t> socketsToRemove;

        for (const auto& pair : connectionLastActive_)
        {
            if (now - pair.second > std::chrono::seconds(120))
            { // 2 min timeout
                socketsToRemove.push_back(pair.first);
            }
        }

        for (socket_t socket : socketsToRemove)
        {
            removeConnection(socket);
        }
    }

    bool running_;
    socket_t listenerSocket_ = SOCKET_ERROR_VALUE;
#if defined(__APPLE__) || defined(__MACH__)
    int kqueueFd_ = -1;
#elif !defined(_WIN32)
    int epollFd_ = -1;
#endif
    std::unordered_map<socket_t, std::shared_ptr<Connection>> connections_;
    std::unordered_map<socket_t, std::chrono::steady_clock::time_point> connectionLastActive_;
    Router router_;
    std::vector<Middleware> globalMiddleware_;
    ErrorHandler errorHandler_;
    std::chrono::steady_clock::time_point lastCleanupTime_ = std::chrono::steady_clock::now();
};

bool Connection::onReadable()
{
    if (state_ != State::READ_REQUEST)
        return true;

    // Read data from socket
    const size_t BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];

    while (true)
    {
        int bytesRead = recv(socket_, buffer, BUFFER_SIZE, 0);

        if (bytesRead > 0)
        {
            readBuffer_.append(buffer, bytesRead);

            if (readBuffer_.size >= 4 &&
                (strstr(readBuffer_.data, "\r\n\r\n") || strstr(readBuffer_.data, "\n\n")))
            {
                state_ = State::PROCESS_REQUEST;
                processRequest();
                return true;
            }

            // This is to check if buffer is becoming too large
            if (readBuffer_.size > 1024 * 1024)
            {                 // 1MB limit
                return false; // Close connection to prevent DoS
            }
        }
        else if (bytesRead == 0)
        {
            // Connection closed by client
            return false;
        }
        else
        {
#ifdef _WIN32
            int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK)
            {
                return true; // No more data available right now
            }
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                return true; // No more data available right now
            }
#endif
            // Error occurred
            return false;
        }
    }

    return true;
}

void Connection::processRequest()
{
    if (state_ != State::PROCESS_REQUEST)
        return;

    // Zero-copy parsing - work directly with the buffer
    readBuffer_.data[readBuffer_.size] = '\0';

    Request request;
    request.setRawRequest(readBuffer_.data);
    request.parse();

    Response response;

    try
    {
        bool continueProcessing = true;
        for (const auto& middleware : middleware_)
        {
            NextFunction next;
            next.setNext([&continueProcessing](const Request& req, Response& res, NextFunction& nf)
                         { continueProcessing = true; });

            middleware(request, response, next);

            if (!continueProcessing)
            {
                break;
            }

            if (response.sent())
            {
                break;
            }
        }

        if (!response.sent())
        {
            bool handled = router_.handle(request, response);

            if (!handled)
            {
                throw NotFoundError("Route not found: " + request.path());
            }
        }
    }
    catch (const std::exception& e)
    {
        if (errorHandler_)
        {
            errorHandler_(e, request, response);
        }
        else
        {
            defaultErrorHandler(e, request, response);
        }
    }

    // Get raw HTTP response
    std::string rawResponse = response.getRawResponse();
    writeBuffer_.clear();
    writeBuffer_.append(rawResponse.c_str(), rawResponse.length());

    // Check if keep-alive
    keepAlive_ = request.header("Connection") != "close";

    // Update connection state
    state_ = State::WRITE_RESPONSE;
    isWriting_ = true;

    // Register for write events
    eventLoop_->updateEvents(socket_, true, true);

    // Try sending response immediately if possible
    onWritable();
}

bool Connection::onWritable()
{
    if (state_ != State::WRITE_RESPONSE || !isWriting_)
        return true;

    // Send data from write buffer
    while (writeBuffer_.size > 0)
    {
        int bytesSent = send(socket_, writeBuffer_.data, writeBuffer_.size, 0);

        if (bytesSent > 0)
        {
            // Remove sent data from buffer
            memmove(writeBuffer_.data, writeBuffer_.data + bytesSent,
                    writeBuffer_.size - bytesSent);
            writeBuffer_.size -= bytesSent;
        }
        else if (bytesSent == 0)
        {
            // Connection closed
            return false;
        }
        else
        {
#ifdef _WIN32
            int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK)
            {
                return true; // Would block, try again later
            }
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                return true; // Would block, try again later
            }
#endif
            // Error occurred
            return false;
        }
    }

    // All data sent
    isWriting_ = false;
    eventLoop_->updateEvents(socket_, true, false);

    if (keepAlive_)
    {
        // Reset for next request
        readBuffer_.clear();
        state_ = State::READ_REQUEST;
        return true;
    }
    else
    {
        // Close connection
        return false;
    }
}

class Server::Impl
{
  public:
    Impl() : port(3000), host("127.0.0.1"), serverSocket(SOCKET_ERROR_VALUE), running(false)
    {
        eventLoop.initialize();
    }

    ~Impl()
    {
        if (running)
        {
            stop();
        }
    }

    bool start(int port, const std::string& host)
    {
        this->port = port;
        this->host = host;

        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == SOCKET_ERROR_VALUE)
        {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }

        int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);

        if (inet_pton(AF_INET, host.c_str(), &(serverAddr.sin_addr)) <= 0)
        {
            std::cerr << "Invalid address" << std::endl;
            close_socket(serverSocket);
            return false;
        }

        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
        {
            std::cerr << "Failed to bind socket" << std::endl;
            close_socket(serverSocket);
            return false;
        }

        if (::listen(serverSocket, SOMAXCONN) < 0)
        {
            std::cerr << "Failed to listen on socket" << std::endl;
            close_socket(serverSocket);
            return false;
        }

        // Event loop
        eventLoop.setRouter(router);
        eventLoop.setGlobalMiddleware(globalMiddleware);
        eventLoop.setErrorHandler(errorHandler);
        eventLoop.addSocket(serverSocket, true);

        running = true;
        std::cout << "Server listening on " << host << ":" << port << std::endl;

        // Run the event loop in the current thread
        // This will block until the server is stopped
        eventLoop.run();

        return true;
    }

    void stop()
    {
        running = false;
        eventLoop.stop();

        if (serverSocket != SOCKET_ERROR_VALUE)
        {
            close_socket(serverSocket);
            serverSocket = SOCKET_ERROR_VALUE;
        }
    }

    // Method to run the server in a background thread
    void startBackgroundThread()
    {
        eventLoopThread = std::thread([this]() { eventLoop.run(); });
    }

    // Method to check if the server is running
    bool isRunning() const { return running; }

    ErrorHandler errorHandler;
    Router router;
    std::vector<Middleware> globalMiddleware;
    bool running;
    int port;
    std::string host;
    socket_t serverSocket;
    EventLoop eventLoop;
    std::thread eventLoopThread;
};

Server::Server() : pimpl(std::make_unique<Impl>()) {}

Server::~Server() {}

Server& Server::configure(int port, const std::string& host)
{
    this->port = port;
    this->host = host;
    return *this;
}

int Server::listen()
{
    if (pimpl->start(port, host))
    {
        return 0;
    }
    return 1;
}

void Server::stop()
{
    pimpl->stop();
}

Server& Server::use(const Middleware& middleware)
{
    pimpl->globalMiddleware.push_back(middleware);
    return *this;
}

Server& Server::use(const std::string& path, const Router& router)
{
    pimpl->router.use(path, router);
    return *this;
}

Server& Server::get(const std::string& path, const RouteHandler& handler)
{
    pimpl->router.get(path, handler);
    return *this;
}

Server& Server::post(const std::string& path, const RouteHandler& handler)
{
    pimpl->router.post(path, handler);
    return *this;
}

Server& Server::put(const std::string& path, const RouteHandler& handler)
{
    pimpl->router.put(path, handler);
    return *this;
}

Server& Server::del(const std::string& path, const RouteHandler& handler)
{
    pimpl->router.del(path, handler);
    return *this;
}

Server& Server::patch(const std::string& path, const RouteHandler& handler)
{
    pimpl->router.patch(path, handler);
    return *this;
}

Server& Server::setErrorHandler(const ErrorHandler& handler)
{
    pimpl->errorHandler = handler;
    return *this;
}

} // namespace boson
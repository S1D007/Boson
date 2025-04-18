#include "boson/server.hpp"
#include "boson/error_handler.hpp"
#include "boson/middleware.hpp"
#include "boson/request.hpp"
#include "boson/response.hpp"
#include "boson/router.hpp"

#include <atomic>
#include <condition_variable>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using socket_t = SOCKET;
#define SOCKET_ERROR_VALUE INVALID_SOCKET
#define close_socket closesocket
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using socket_t = int;
#define SOCKET_ERROR_VALUE (-1)
#define close_socket close
#endif

namespace boson
{

class Server::Impl
{
  public:
    Impl() : running(false), port(3000), host("127.0.0.1"), serverSocket(SOCKET_ERROR_VALUE) {}

    ~Impl()
    {
        if (running)
        {
            stop();
        }
    }

    bool initialize()
    {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            std::cerr << "Failed to initialize Winsock" << std::endl;
            return false;
        }
#endif
        return true;
    }

    void cleanup()
    {
#ifdef _WIN32
        WSACleanup();
#endif
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

        running = true;

        startWorkerThreads();

        std::cout << "Server listening on " << host << ":" << port << std::endl;

        acceptLoop();

        return true;
    }

    void stop()
    {
        running = false;

        if (serverSocket != SOCKET_ERROR_VALUE)
        {
            close_socket(serverSocket);
            serverSocket = SOCKET_ERROR_VALUE;
        }

        std::unique_lock<std::mutex> lock(queueMutex);
        queueCondition.notify_all();
        lock.unlock();

        for (auto& thread : workerThreads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }

        workerThreads.clear();

        cleanup();
    }

    void acceptLoop()
    {
        while (running)
        {
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);

            socket_t clientSocket =
                accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
            if (clientSocket == SOCKET_ERROR_VALUE)
            {
                if (!running)
                {
                    break;
                }
                std::cerr << "Failed to accept connection" << std::endl;
                continue;
            }

            {
                std::lock_guard<std::mutex> lock(queueMutex);
                clientQueue.push(clientSocket);
            }

            queueCondition.notify_one();
        }
    }

    void startWorkerThreads()
    {
        unsigned int numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0)
        {
            numThreads = 4;
        }

        for (unsigned int i = 0; i < numThreads; i++)
        {
            workerThreads.emplace_back(&Impl::workerThread, this);
        }
    }

    void workerThread()
    {
        while (running)
        {
            socket_t clientSocket;

            {
                std::unique_lock<std::mutex> lock(queueMutex);
                while (running && clientQueue.empty())
                {
                    queueCondition.wait(lock);
                }

                if (!running)
                {
                    break;
                }

                clientSocket = clientQueue.front();
                clientQueue.pop();
            }

            handleClient(clientSocket);

            close_socket(clientSocket);
        }
    }

    void handleClient(socket_t clientSocket)
    {
        constexpr int headerBufferSize = 8192;
        char headerBuffer[headerBufferSize];
        std::string requestData;
        
        int bytesRead = recv(clientSocket, headerBuffer, headerBufferSize - 1, 0);
        if (bytesRead <= 0) {
            return;
        }
        
        headerBuffer[bytesRead] = '\0';
        requestData.append(headerBuffer, bytesRead);
        
        size_t headerEnd = requestData.find("\r\n\r\n");
        if (headerEnd == std::string::npos) {
            return;
        }
        
        size_t contentLengthPos = requestData.find("Content-Length:");
        size_t bodyLength = 0;
        
        if (contentLengthPos != std::string::npos) {
            size_t valueStart = requestData.find_first_not_of(" \t", contentLengthPos + 15);
            size_t valueEnd = requestData.find_first_of("\r\n", valueStart);
            if (valueStart != std::string::npos && valueEnd != std::string::npos) {
                try {
                    bodyLength = std::stoul(requestData.substr(valueStart, valueEnd - valueStart));
                } catch (...) { }
            }
        }
        
        size_t bodyRead = requestData.size() - (headerEnd + 4);
        
        if (bodyLength > bodyRead) {
            size_t remainingBytes = bodyLength - bodyRead;
            
            char* bodyBuffer = new char[8192];
            while (remainingBytes > 0) {
                size_t chunkSize = std::min(remainingBytes, (size_t)8192);
                bytesRead = recv(clientSocket, bodyBuffer, static_cast<int>(chunkSize), 0);
                
                if (bytesRead <= 0) {
                    break;
                }
                
                requestData.append(bodyBuffer, bytesRead);
                remainingBytes -= bytesRead;
            }
            delete[] bodyBuffer;
        }

        Request request;
        request.setRawRequest(requestData);
        request.parse();
        
        std::string contentType = request.header("Content-Type");
        if (contentType.find("multipart/form-data") != std::string::npos && bodyLength > 0) {
            std::string body = requestData.substr(headerEnd + 4);
            request.setBody(body);
        }

        Response response;

        try
        {

            bool continueProcessing = true;
            for (const auto& middleware : globalMiddleware)
            {
                NextFunction next;
                next.setNext([&continueProcessing](const Request&, Response&, NextFunction&)
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
                bool handled = router.handle(request, response);

                if (!handled)
                {
                    throw NotFoundError("Route not found: " + request.path());
                }
            }
        }
        catch (const std::exception& e)
        {

            if (errorHandler)
            {
                errorHandler(e, request, response);
            }
            else
            {
                defaultErrorHandler(e, request, response);
            }
        }

        std::string rawResponse = response.getRawResponse();
        send(clientSocket, rawResponse.c_str(), rawResponse.length(), 0);
    }

    ErrorHandler errorHandler;
    Router router;
    std::vector<Middleware> globalMiddleware;
    std::atomic<bool> running;
    int port;
    std::string host;
    socket_t serverSocket;

    std::vector<std::thread> workerThreads;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::queue<socket_t> clientQueue;
};

Server::Server() : pimpl(std::make_unique<Impl>())
{
    pimpl->initialize();
}

Server::~Server() {}

Server& Server::configure(int port, const std::string& host)
{
    this->port = port;
    this->host = host;
    return *this;
}

int Server::listen()
{
    return pimpl->start(port, host) ? 0 : 1;
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
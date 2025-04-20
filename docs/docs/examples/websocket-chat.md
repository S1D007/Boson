---
sidebar_position: 2
title: WebSocket Chat App
---

# Building a Real-time Chat with WebSockets

This example demonstrates how to build a real-time chat application using Boson's WebSocket support. We'll create a simple chat server that allows multiple clients to connect and exchange messages.

## Project Structure

```
websocket-chat/
├── CMakeLists.txt
├── public/
│   ├── index.html
│   ├── css/
│   │   └── style.css
│   └── js/
│       └── chat.js
└── src/
    ├── main.cpp
    ├── chat_server.hpp
    ├── chat_server.cpp
    ├── chat_room.hpp
    ├── chat_room.cpp
    ├── chat_participant.hpp
    └── chat_participant.cpp
```

## Step 1: Setting Up the WebSocket Chat Server

First, let's define the core components of our chat server:

```cpp
// chat_participant.hpp
#pragma once
#include <boson/boson.hpp>
#include <string>
#include <memory>
#include <set>

// Forward declarations
class ChatRoom;

// Represents a single connected client
class ChatParticipant : public std::enable_shared_from_this<ChatParticipant> {
public:
    using Pointer = std::shared_ptr<ChatParticipant>;

    ChatParticipant(boson::WebSocket socket);
    ~ChatParticipant();

    // Start listening for messages
    void start();

    // Send a message to this participant
    void deliver(const std::string& message);

    // Get the participant's username
    std::string username() const;

    // Set the chat room this participant is in
    void setRoom(std::shared_ptr<ChatRoom> room);

private:
    // Handle incoming messages
    void handleMessage(const std::string& message);

    // Send the welcome message
    void sendWelcome();

    // Parse a command
    void handleCommand(const std::string& command);

    // Parse incoming message
    void processMessage(const std::string& message);

    boson::WebSocket socket_;
    std::string username_;
    std::weak_ptr<ChatRoom> room_;
    bool hasJoined_;
};

// chat_room.hpp
#pragma once
#include "chat_participant.hpp"
#include <set>
#include <string>
#include <queue>
#include <mutex>

class ChatRoom {
public:
    // Add a participant to the room
    void join(ChatParticipant::Pointer participant);

    // Remove a participant from the room
    void leave(ChatParticipant::Pointer participant);

    // Deliver a message to all participants in the room
    void broadcast(const std::string& message, ChatParticipant::Pointer sender = nullptr);

    // Get number of participants
    size_t size() const;

private:
    std::set<ChatParticipant::Pointer> participants_;
    std::mutex mutex_;
};

// chat_server.hpp
#pragma once
#include <boson/boson.hpp>
#include <memory>
#include <string>
#include "chat_room.hpp"

class ChatServer {
public:
    ChatServer(boson::Server& server);

    // Start the chat server
    void start();

private:
    // Handle a new WebSocket connection
    void handleConnection(boson::WebSocket socket);

    boson::Server& server_;
    std::shared_ptr<ChatRoom> room_;
};
```

Now let's implement these classes:

```cpp
// chat_participant.cpp
#include "chat_participant.hpp"
#include "chat_room.hpp"
#include <nlohmann/json.hpp>
#include <iostream>

ChatParticipant::ChatParticipant(boson::WebSocket socket)
    : socket_(std::move(socket)), username_("Anonymous"), hasJoined_(false) {
}

ChatParticipant::~ChatParticipant() {
    auto room = room_.lock();
    if (room) {
        room->leave(shared_from_this());
    }
}

void ChatParticipant::start() {
    // Set up message handler
    socket_.onMessage([self = shared_from_this()](const std::string& message) {
        self->handleMessage(message);
    });
    
    // Set up close handler
    socket_.onClose([self = shared_from_this()](int code, const std::string& reason) {
        std::cout << "WebSocket closed: " << reason << " (" << code << ")" << std::endl;
        auto room = self->room_.lock();
        if (room) {
            room->leave(self);
            room->broadcast(self->username_ + " has left the chat", self);
        }
    });
    
    // Set up error handler
    socket_.onError([self = shared_from_this()](const std::string& error) {
        std::cerr << "WebSocket error: " << error << std::endl;
    });
    
    // Send the welcome message
    sendWelcome();
}

void ChatParticipant::deliver(const std::string& message) {
    socket_.send(message);
}

std::string ChatParticipant::username() const {
    return username_;
}

void ChatParticipant::setRoom(std::shared_ptr<ChatRoom> room) {
    room_ = room;
}

void ChatParticipant::handleMessage(const std::string& message) {
    try {
        // Check if it's a command (starts with '/')
        if (!message.empty() && message[0] == '/') {
            handleCommand(message);
            return;
        }
        
        // Process regular message
        if (hasJoined_) {
            processMessage(message);
        } else {
            // User needs to join first
            deliver(R"({"type":"error","message":"Please join the chat first using /join <username>"})");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling message: " << e.what() << std::endl;
        deliver(R"({"type":"error","message":"Invalid message format"})");
    }
}

void ChatParticipant::sendWelcome() {
    nlohmann::json welcome = {
        {"type", "system"},
        {"message", "Welcome to the Boson Chat Server! Use /join <username> to join."}
    };
    deliver(welcome.dump());
}

void ChatParticipant::handleCommand(const std::string& command) {
    // Parse the command
    std::string cmd = command.substr(1); // Remove the '/'
    size_t spacePos = cmd.find(' ');
    std::string cmdName = (spacePos != std::string::npos) ? cmd.substr(0, spacePos) : cmd;
    std::string args = (spacePos != std::string::npos) ? cmd.substr(spacePos + 1) : "";
    
    if (cmdName == "join") {
        // Handle join command
        if (args.empty()) {
            deliver(R"({"type":"error","message":"Please provide a username"})");
            return;
        }
        
        username_ = args;
        hasJoined_ = true;
        
        // Notify user
        nlohmann::json response = {
            {"type", "system"},
            {"message", "You have joined as '" + username_ + "'"}
        };
        deliver(response.dump());
        
        // Notify everyone
        auto room = room_.lock();
        if (room) {
            room->broadcast(username_ + " has joined the chat", shared_from_this());
        }
    }
    else if (cmdName == "name" || cmdName == "nick") {
        // Change username
        if (!hasJoined_) {
            deliver(R"({"type":"error","message":"Please join first using /join <username>"})");
            return;
        }
        
        if (args.empty()) {
            deliver(R"({"type":"error","message":"Please provide a new username"})");
            return;
        }
        
        std::string oldName = username_;
        username_ = args;
        
        // Notify user
        nlohmann::json response = {
            {"type", "system"},
            {"message", "You are now known as '" + username_ + "'"}
        };
        deliver(response.dump());
        
        // Notify everyone
        auto room = room_.lock();
        if (room) {
            room->broadcast(oldName + " is now known as " + username_, shared_from_this());
        }
    }
    else if (cmdName == "help") {
        // Show help
        nlohmann::json response = {
            {"type", "system"},
            {"message", "Available commands:\n" 
                        "/join <username> - Join the chat\n"
                        "/name <new_name> - Change your username\n"
                        "/help - Show this help message"}
        };
        deliver(response.dump());
    }
    else {
        // Unknown command
        nlohmann::json response = {
            {"type", "error"},
            {"message", "Unknown command: " + cmdName + ". Type /help for available commands."}
        };
        deliver(response.dump());
    }
}

void ChatParticipant::processMessage(const std::string& message) {
    auto room = room_.lock();
    if (!room) {
        return;
    }
    
    try {
        // Create message JSON
        nlohmann::json msgJson = {
            {"type", "message"},
            {"username", username_},
            {"message", message},
            {"timestamp", std::chrono::system_clock::now().time_since_epoch().count()}
        };
        
        // Broadcast to all participants
        room->broadcast(msgJson.dump(), shared_from_this());
    } catch (const std::exception& e) {
        std::cerr << "Error processing message: " << e.what() << std::endl;
    }
}

// chat_room.cpp
#include "chat_room.hpp"
#include <iostream>

void ChatRoom::join(ChatParticipant::Pointer participant) {
    std::lock_guard<std::mutex> lock(mutex_);
    participants_.insert(participant);
    participant->setRoom(shared_from_this());
}

void ChatRoom::leave(ChatParticipant::Pointer participant) {
    std::lock_guard<std::mutex> lock(mutex_);
    participants_.erase(participant);
}

void ChatRoom::broadcast(const std::string& message, ChatParticipant::Pointer sender) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& participant : participants_) {
        if (participant != sender) {
            try {
                participant->deliver(message);
            } catch (const std::exception& e) {
                std::cerr << "Error delivering message: " << e.what() << std::endl;
            }
        }
    }
}

size_t ChatRoom::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return participants_.size();
}

// chat_server.cpp
#include "chat_server.hpp"
#include <iostream>

ChatServer::ChatServer(boson::Server& server) : server_(server) {
    room_ = std::make_shared<ChatRoom>();
}

void ChatServer::start() {
    // Handle WebSocket connections at the /chat path
    server_.ws("/chat", [this](boson::WebSocket socket) {
        handleConnection(std::move(socket));
    });
    
    std::cout << "Chat server started on /chat" << std::endl;
}

void ChatServer::handleConnection(boson::WebSocket socket) {
    std::cout << "New chat connection from: " << socket.remoteAddress() << std::endl;
    
    // Create new participant and start it
    auto participant = std::make_shared<ChatParticipant>(std::move(socket));
    room_->join(participant);
    participant->start();
}
```

## Step 2: Creating the Main Application

Now let's set up the main application that hosts both the WebSocket server and serves the static HTML/JS/CSS files for the chat client:

```cpp
// main.cpp
#include <boson/boson.hpp>
#include <iostream>
#include <string>
#include "chat_server.hpp"

int main() {
    try {
        // Initialize Boson framework
        boson::initialize();
        
        // Create server instance
        boson::Server app;
        
        // Create and start chat server
        ChatServer chatServer(app);
        chatServer.start();
        
        // Serve static files for the chat client
        app.staticDir("/", "./public");
        
        // Info route
        app.get("/info", [](const boson::Request& req, boson::Response& res) {
            res.jsonObject({
                {"name", "Boson WebSocket Chat"},
                {"version", "1.0.0"},
                {"connections", 0},  // In a real app, get this from ChatServer
                {"uptime", 0}        // In a real app, calculate uptime
            });
        });
        
        // Configure and start the server
        app.configure(3000, "127.0.0.1");
        std::cout << "Chat server running at http://127.0.0.1:3000" << std::endl;
        return app.listen();
    }
    catch (const std::exception& e) {
        std::cerr << "Server initialization failed: " << e.what() << std::endl;
        return 1;
    }
}
```

## Step 3: Creating the Front-end Client

Now let's create the HTML, CSS, and JavaScript for the chat client:

```html
<!-- public/index.html -->
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Boson Chat</title>
    <link rel="stylesheet" href="/css/style.css">
</head>
<body>
    <div class="chat-container">
        <header class="chat-header">
            <h1>Boson Chat</h1>
            <div id="connection-status" class="connection-status">Disconnected</div>
        </header>
        
        <main class="chat-main">
            <div class="chat-messages" id="chat-messages"></div>
        </main>
        
        <div class="chat-form-container">
            <form id="chat-form">
                <input
                    id="msg"
                    type="text"
                    placeholder="Type a message..."
                    autocomplete="off"
                    required
                />
                <button class="btn">Send</button>
            </form>
        </div>
    </div>

    <script src="/js/chat.js"></script>
</body>
</html>
```

```css
/* public/css/style.css */
:root {
    --dark-color-a: #3c59a8;
    --dark-color-b: #526db0;
    --light-color: #e6e9ff;
    --success-color: #5cb85c;
    --error-color: #d9534f;
}

* {
    box-sizing: border-box;
    margin: 0;
    padding: 0;
}

body {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    font-size: 16px;
    background: #f5f7ff;
    margin: 20px;
}

.chat-container {
    max-width: 800px;
    background: #fff;
    margin: 30px auto;
    overflow: hidden;
    border-radius: 10px;
    box-shadow: 0 5px 15px rgba(0, 0, 0, 0.1);
}

.chat-header {
    background: var(--dark-color-a);
    color: #fff;
    padding: 15px;
    display: flex;
    align-items: center;
    justify-content: space-between;
}

.chat-header h1 {
    font-size: 1.5rem;
}

.connection-status {
    padding: 5px 10px;
    background: var(--error-color);
    color: white;
    border-radius: 15px;
    font-size: 0.8rem;
}

.connection-status.connected {
    background: var(--success-color);
}

.chat-main {
    display: grid;
    grid-template-rows: 1fr;
    height: 500px;
    background: #f5f7ff;
}

.chat-messages {
    padding: 20px;
    max-height: 500px;
    overflow-y: auto;
}

.chat-messages .message {
    padding: 10px;
    margin-bottom: 15px;
    background-color: var(--light-color);
    border-radius: 5px;
}

.chat-messages .message .meta {
    font-size: 0.9rem;
    font-weight: bold;
    color: var(--dark-color-b);
    margin-bottom: 7px;
    display: flex;
    justify-content: space-between;
}

.chat-messages .message.system {
    background-color: #ebf1ff;
    color: #4a5b8c;
    font-style: italic;
}

.chat-messages .message.error {
    background-color: #ffebeb;
    color: #8c4a4a;
}

.chat-form-container {
    padding: 15px;
    background-color: var(--dark-color-a);
}

.chat-form-container form {
    display: flex;
}

.chat-form-container input[type='text'] {
    flex: 1;
    padding: 10px;
    border: none;
    border-radius: 5px 0 0 5px;
    outline: none;
    font-size: 1rem;
}

.chat-form-container button {
    padding: 10px 15px;
    border: none;
    border-radius: 0 5px 5px 0;
    background: var(--dark-color-b);
    color: #fff;
    font-size: 1rem;
    cursor: pointer;
    transition: background 0.2s;
}

.chat-form-container button:hover {
    background: #445c9e;
}
```

```javascript
// public/js/chat.js
document.addEventListener('DOMContentLoaded', () => {
    const chatForm = document.getElementById('chat-form');
    const chatMessages = document.getElementById('chat-messages');
    const connectionStatus = document.getElementById('connection-status');
    let socket;
    
    // Connect to WebSocket server
    function connectWebSocket() {
        // Create WebSocket connection
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const host = window.location.host;
        const wsUrl = `${protocol}//${host}/chat`;
        
        connectionStatus.textContent = 'Connecting...';
        connectionStatus.className = 'connection-status';
        
        socket = new WebSocket(wsUrl);
        
        // Connection opened
        socket.addEventListener('open', (event) => {
            connectionStatus.textContent = 'Connected';
            connectionStatus.className = 'connection-status connected';
            
            console.log('Connected to chat server');
        });
        
        // Listen for messages
        socket.addEventListener('message', (event) => {
            try {
                const data = JSON.parse(event.data);
                displayMessage(data);
            } catch (err) {
                console.error('Error parsing message:', err);
                displayMessage({
                    type: 'error',
                    message: 'Failed to parse message from server'
                });
            }
        });
        
        // Connection closed
        socket.addEventListener('close', (event) => {
            connectionStatus.textContent = 'Disconnected';
            connectionStatus.className = 'connection-status';
            
            console.log('Disconnected from chat server');
            
            // Try to reconnect after 5 seconds
            setTimeout(connectWebSocket, 5000);
        });
        
        // Connection error
        socket.addEventListener('error', (event) => {
            connectionStatus.textContent = 'Connection Error';
            connectionStatus.className = 'connection-status';
            
            console.error('WebSocket error:', event);
            displayMessage({
                type: 'error',
                message: 'Connection error. Please try again later.'
            });
        });
    }
    
    // Display message to DOM
    function displayMessage(data) {
        const div = document.createElement('div');
        div.classList.add('message');
        
        if (data.type === 'system') {
            div.classList.add('system');
            div.innerHTML = `<p>${data.message}</p>`;
        } 
        else if (data.type === 'error') {
            div.classList.add('error');
            div.innerHTML = `<p>${data.message}</p>`;
        } 
        else {
            // Format timestamp if available
            let timeStr = '';
            if (data.timestamp) {
                const date = new Date(Number(data.timestamp) / 1000000);
                timeStr = date.toLocaleTimeString();
            }
            
            div.innerHTML = `
                <div class="meta">
                    <span>${data.username}</span>
                    <span>${timeStr}</span>
                </div>
                <p>${data.message}</p>
            `;
        }
        
        chatMessages.appendChild(div);
        
        // Scroll to bottom
        chatMessages.scrollTop = chatMessages.scrollHeight;
    }
    
    // Message submit
    chatForm.addEventListener('submit', (e) => {
        e.preventDefault();
        
        // Get message text
        const msg = document.getElementById('msg').value;
        
        if (!socket || socket.readyState !== WebSocket.OPEN) {
            displayMessage({
                type: 'error',
                message: 'Not connected to the chat server'
            });
            return;
        }
        
        // Send message to server
        socket.send(msg);
        
        // Clear input
        document.getElementById('msg').value = '';
        document.getElementById('msg').focus();
    });
    
    // Start connection
    connectWebSocket();
});
```

## Step 4: Setting up the CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.14)
project(boson-websocket-chat VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boson package
find_package(Boson REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Collect source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link against Boson
target_link_libraries(${PROJECT_NAME} PRIVATE Boson::Boson)

# Copy public directory to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/public DESTINATION ${CMAKE_BINARY_DIR})
```

## Running the Application

Build and run the application:

```bash
mkdir build && cd build
cmake ..
make
./boson-websocket-chat
```

Then open your browser to `http://localhost:3000` to access the chat client. You can open multiple browser windows to simulate multiple users chatting.

## WebSocket API Explanation

Boson provides a simple API for WebSocket support:

### Server-side WebSockets

1. **Creating a WebSocket endpoint**:
   ```cpp
   server.ws("/path", [](boson::WebSocket socket) {
       // Handle new WebSocket connection
   });
   ```

2. **WebSocket event handlers**:
   ```cpp
   socket.onMessage([](const std::string& message) {
       // Handle incoming message
   });
   
   socket.onClose([](int code, const std::string& reason) {
       // Handle connection close
   });
   
   socket.onError([](const std::string& error) {
       // Handle error
   });
   ```

3. **Sending messages**:
   ```cpp
   socket.send("Hello, WebSocket!");
   ```

### Client-side WebSockets

The client uses standard browser WebSocket API:

1. **Creating a connection**:
   ```javascript
   const socket = new WebSocket("ws://localhost:3000/chat");
   ```

2. **Event listeners**:
   ```javascript
   socket.addEventListener('open', (event) => {
       // Connection opened
   });
   
   socket.addEventListener('message', (event) => {
       // Message received
   });
   
   socket.addEventListener('close', (event) => {
       // Connection closed
   });
   
   socket.addEventListener('error', (event) => {
       // Connection error
   });
   ```

3. **Sending messages**:
   ```javascript
   socket.send("Hello from client!");
   ```

## Summary

This example demonstrates how to build a real-time chat application using Boson's WebSocket support. Key concepts covered include:

1. **WebSocket Handling**: Setting up WebSocket endpoints on the server
2. **Event-Based Communication**: Handling WebSocket events like connection, message, and disconnection
3. **Chat Room Management**: Managing participants in a chat room
4. **Message Broadcasting**: Sending messages to multiple clients
5. **Client-Side Integration**: Creating a responsive web client using HTML, CSS, and JavaScript
6. **Command Handling**: Processing chat commands with a slash (/) prefix
7. **Error Handling**: Gracefully handling connection and message errors

This pattern can be extended to build various real-time applications like collaborative tools, live dashboards, or multiplayer games.
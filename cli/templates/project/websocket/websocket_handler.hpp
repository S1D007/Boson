#pragma once

#include <boson/websocket.hpp>
#include <string>
#include <functional>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <iostream>

/**
 * @class WebSocketHandler
 * @brief Handles WebSocket connections and messages
 */
class WebSocketHandler {
public:
    /**
     * @brief Default constructor
     */
    WebSocketHandler() = default;
    
    /**
     * @brief Handle a new connection
     * @param ws The WebSocket connection
     */
    void onConnect(boson::WebSocket& ws) {
        std::string clientId = generateClientId();
        
        std::lock_guard<std::mutex> lock(clientsMutex_);
        clients_[clientId] = ws.shared_from_this();
        
        
        ws.onMessage([this, clientId](const std::string& message) {
            this->handleMessage(clientId, message);
        });
        
        
        ws.onClose([this, clientId]() {
            this->handleDisconnect(clientId);
        });
        
        
        ws.send("{\"type\":\"welcome\",\"message\":\"Welcome to the chat!\",\"clientId\":\"" + clientId + "\"}");
        
        
        broadcast("{\"type\":\"system\",\"message\":\"A new user has joined the chat.\",\"clientId\":\"" + clientId + "\"}");
        
        std::cout << "Client connected: " << clientId << std::endl;
    }
    
    /**
     * @brief Handle a disconnect
     * @param clientId The ID of the client
     */
    void handleDisconnect(const std::string& clientId) {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        clients_.erase(clientId);
        
        
        broadcast("{\"type\":\"system\",\"message\":\"A user has left the chat.\",\"clientId\":\"" + clientId + "\"}");
        
        std::cout << "Client disconnected: " << clientId << std::endl;
    }
    
    /**
     * @brief Handle an incoming message
     * @param clientId The ID of the client
     * @param message The message received
     */
    void handleMessage(const std::string& clientId, const std::string& message) {
        
        std::string broadcastMessage = "{\"type\":\"chat\",\"message\":" + message + ",\"clientId\":\"" + clientId + "\"}";
        broadcast(broadcastMessage);
        
        std::cout << "Message from " << clientId << ": " << message << std::endl;
    }
    
    /**
     * @brief Broadcast a message to all clients
     * @param message The message to broadcast
     */
    void broadcast(const std::string& message) {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        for (const auto& client : clients_) {
            try {
                client.second->send(message);
            } catch (const std::exception& e) {
                std::cerr << "Error sending to client " << client.first 
                          << ": " << e.what() << std::endl;
            }
        }
    }

private:
    /**
     * @brief Generate a unique client ID
     * @return A unique client ID
     */
    std::string generateClientId() const {
        static int counter = 0;
        return "user_" + std::to_string(++counter);
    }
    
    std::unordered_map<std::string, std::shared_ptr<boson::WebSocket>> clients_;
    std::mutex clientsMutex_;
};
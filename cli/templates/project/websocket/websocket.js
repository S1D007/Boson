document.addEventListener('DOMContentLoaded', () => {
    const statusElement = document.getElementById('status');
    const messagesElement = document.getElementById('messages');
    const messageForm = document.getElementById('message-form');
    const messageInput = document.getElementById('message');
    const sendButton = document.getElementById('send-button');
    
    let socket;
    let clientId;
    
    function connect() {
        
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const host = window.location.host;
        socket = new WebSocket(protocol + '//' + host + '/ws');
        
        
        socket.addEventListener('open', () => {
            statusElement.textContent = 'Connected';
            statusElement.classList.remove('disconnected');
            statusElement.classList.add('connected');
            
            messageInput.disabled = false;
            sendButton.disabled = false;
            
            addMessage('System', 'Connected to the chat server.');
        });
        
        
        socket.addEventListener('message', (event) => {
            try {
                const data = JSON.parse(event.data);
                
                switch (data.type) {
                    case 'welcome':
                        clientId = data.clientId;
                        addMessage('System', data.message);
                        break;
                    
                    case 'system':
                        addMessage('System', data.message);
                        break;
                    
                    case 'chat':
                        const sender = data.clientId === clientId ? 'You' : 'User ' + data.clientId;
                        addMessage(sender, data.message);
                        break;
                    
                    default:
                        console.log('Unknown message type:', data);
                }
            } catch (error) {
                console.error('Error parsing message:', error);
            }
        });
        
        
        socket.addEventListener('close', () => {
            statusElement.textContent = 'Disconnected';
            statusElement.classList.remove('connected');
            statusElement.classList.add('disconnected');
            
            messageInput.disabled = true;
            sendButton.disabled = true;
            
            addMessage('System', 'Disconnected from the chat server.');
            
            
            setTimeout(() => {
                addMessage('System', 'Attempting to reconnect...');
                connect();
            }, 5000);
        });
        
        
        socket.addEventListener('error', (error) => {
            console.error('WebSocket error:', error);
            addMessage('System', 'Error connecting to the chat server.');
        });
    }
    
    
    function addMessage(sender, text) {
        const messageElement = document.createElement('div');
        messageElement.classList.add('message');
        
        if (sender === 'System') {
            messageElement.classList.add('system-message');
        }
        
        messageElement.textContent = sender + ': ' + text;
        messagesElement.appendChild(messageElement);
        
        
        messagesElement.scrollTop = messagesElement.scrollHeight;
    }
    
    
    messageForm.addEventListener('submit', (event) => {
        event.preventDefault();
        
        const message = messageInput.value.trim();
        if (!message) return;
        
        
        if (socket && socket.readyState === WebSocket.OPEN) {
            socket.send(JSON.stringify(message));
            messageInput.value = '';
        } else {
            addMessage('System', 'Not connected to the chat server.');
        }
    });
    
    connect();
});
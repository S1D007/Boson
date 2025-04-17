// Simple JavaScript file to demonstrate static file serving

document.addEventListener('DOMContentLoaded', function() {
    console.log('Static file serving is working!');
    
    const infoBox = document.querySelector('.info-box');
    
    const button = document.createElement('button');
    button.textContent = 'Click me!';
    button.style.padding = '8px 16px';
    button.style.backgroundColor = '#3498db';
    button.style.color = 'white';
    button.style.border = 'none';
    button.style.borderRadius = '4px';
    button.style.cursor = 'pointer';
    button.style.marginTop = '10px';
    
    button.addEventListener('click', function() {
        const timestamp = new Date().toLocaleTimeString();
        const message = document.createElement('p');
        message.textContent = `Button clicked at ${timestamp}`;
        message.style.color = '#3498db';
        infoBox.appendChild(message);
    });
    
    infoBox.appendChild(button);
});
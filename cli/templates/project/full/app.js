document.addEventListener('DOMContentLoaded', () => {
    console.log('Boson application loaded');
    
    
    const links = document.querySelectorAll('a');
    links.forEach(link => {
        link.addEventListener('click', (e) => {
            
            if (link.getAttribute('href').startsWith('/api')) {
                e.preventDefault();
                
                
                fetch(link.getAttribute('href'))
                    .then(response => response.json())
                    .then(data => {
                        console.log('API response:', data);
                        
                    })
                    .catch(error => {
                        console.error('API error:', error);
                    });
            }
        });
    });
});
document.addEventListener('DOMContentLoaded', function() {
    // Initialize application components
    initializeUserInterface();
    
    // Add event listeners
    setupEventHandlers();
});

function initializeUserInterface() {
    console.log('{{.ProjectName}} application initialized');
    
    // Highlight current navigation item
    const currentPath = window.location.pathname;
    document.querySelectorAll('nav ul li a').forEach(link => {
        if (link.getAttribute('href') === currentPath) {
            link.parentElement.classList.add('active');
        }
    });
    
    // Check if we're on a users page and load user data if needed
    if (window.location.pathname === '/users') {
        loadUserData();
    }
}

function setupEventHandlers() {
    // Add event listeners for interactive elements
    const userForm = document.getElementById('user-form');
    if (userForm) {
        userForm.addEventListener('submit', handleUserFormSubmit);
    }
    
    // Add click handlers for delete buttons
    document.querySelectorAll('.delete-user').forEach(button => {
        button.addEventListener('click', handleDeleteUser);
    });
}

// Load user data from API
async function loadUserData() {
    try {
        const usersContainer = document.getElementById('user-list');
        if (!usersContainer) return;
        
        // Show loading state
        usersContainer.innerHTML = '<p>Loading users...</p>';
        
        const response = await fetch('/api/users');
        const data = await response.json();
        
        if (data.users && data.users.length > 0) {
            usersContainer.innerHTML = '';
            
            // Render each user
            data.users.forEach(user => {
                const userCard = createUserCard(user);
                usersContainer.appendChild(userCard);
            });
        } else {
            usersContainer.innerHTML = '<p>No users found.</p>';
        }
    } catch (error) {
        console.error('Error loading users:', error);
        const usersContainer = document.getElementById('user-list');
        if (usersContainer) {
            usersContainer.innerHTML = '<p class="error">Failed to load users. Please try again later.</p>';
        }
    }
}

// Create HTML for a user card
function createUserCard(user) {
    const card = document.createElement('div');
    card.className = 'user-card';
    card.dataset.userId = user.id;
    
    card.innerHTML = `
        <h3>${user.name}</h3>
        <p class="email">${user.email}</p>
        <p>Status: <span class="status ${user.active ? 'active' : 'inactive'}">${user.active ? 'Active' : 'Inactive'}</span></p>
        <div class="user-actions">
            <a href="/users/${user.id}" class="btn btn-primary">View</a>
            <a href="/users/${user.id}/edit" class="btn btn-secondary">Edit</a>
            <button class="btn btn-danger delete-user" data-user-id="${user.id}">Delete</button>
        </div>
    `;
    
    // Add delete handler to the button
    card.querySelector('.delete-user').addEventListener('click', handleDeleteUser);
    
    return card;
}

// Handle user form submissions
async function handleUserFormSubmit(event) {
    event.preventDefault();
    
    const form = event.target;
    const userId = form.dataset.userId; // If exists, we're updating
    const isUpdate = !!userId;
    
    // Collect form data
    const formData = {
        name: form.elements.name.value,
        email: form.elements.email.value,
        active: form.elements.active ? form.elements.active.checked : true
    };
    
    try {
        // Show loading state
        const submitBtn = form.querySelector('button[type="submit"]');
        const originalBtnText = submitBtn.textContent;
        submitBtn.textContent = 'Saving...';
        submitBtn.disabled = true;
        
        // Determine URL and method based on if we're creating or updating
        const url = isUpdate ? `/api/users/${userId}` : '/api/users';
        const method = isUpdate ? 'PUT' : 'POST';
        
        // Send request
        const response = await fetch(url, {
            method: method,
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(formData)
        });
        
        if (!response.ok) {
            throw new Error('Server returned an error');
        }
        
        const result = await response.json();
        
        // Redirect to user list or show success message
        window.location.href = '/users';
    } catch (error) {
        console.error('Error saving user:', error);
        const errorContainer = form.querySelector('.error-message');
        if (errorContainer) {
            errorContainer.textContent = 'Error saving user. Please try again.';
            errorContainer.style.display = 'block';
        }
    } finally {
        // Reset button state
        const submitBtn = form.querySelector('button[type="submit"]');
        submitBtn.textContent = originalBtnText;
        submitBtn.disabled = false;
    }
}

// Handle user deletion
async function handleDeleteUser(event) {
    const userId = event.target.dataset.userId;
    if (!userId) return;
    
    if (!confirm('Are you sure you want to delete this user?')) {
        return;
    }
    
    try {
        // Send delete request
        const response = await fetch(`/api/users/${userId}`, {
            method: 'DELETE'
        });
        
        if (!response.ok) {
            throw new Error('Server returned an error');
        }
        
        // Remove user card from UI
        const userCard = document.querySelector(`.user-card[data-user-id="${userId}"]`);
        if (userCard) {
            userCard.remove();
        }
        
        // Reload users if we're on the users page
        if (window.location.pathname === '/users') {
            loadUserData();
        } else {
            // If on a specific user page, redirect back to list
            window.location.href = '/users';
        }
    } catch (error) {
        console.error('Error deleting user:', error);
        alert('Failed to delete user. Please try again.');
    }
}
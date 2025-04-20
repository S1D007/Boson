---
sidebar_position: 4
title: Request and Response
---

# Request and Response Objects

The Request and Response objects are the core components you'll interact with when handling HTTP requests in Boson. Understanding these objects is essential for building effective web applications.

## The Request Object

The `boson::Request` object represents an incoming HTTP request and provides methods to access its data, including headers, URL parameters, query parameters, and body content.

### Request Properties

```cpp
void handleRequest(const boson::Request& req, boson::Response& res) {
    // Get request method (GET, POST, PUT, DELETE, etc.)
    std::string method = req.method();
    
    // Get request path (/api/users, /products/123, etc.)
    std::string path = req.path();
    
    // Get original URL including query string (/search?q=term&page=1)
    std::string url = req.url();
    
    // Get HTTP protocol version
    std::string httpVersion = req.httpVersion();
    
    // Get client IP address
    std::string ip = req.ip();
    
    // Get content type
    std::string contentType = req.contentType();
}
```

### Headers

Access request headers:

```cpp
void handleRequest(const boson::Request& req, boson::Response& res) {
    // Get a specific header (case-insensitive)
    std::string userAgent = req.header("User-Agent");
    
    // Get a header with a default value if not present
    std::string accept = req.header("Accept", "application/json");
    
    // Check if a header exists
    bool hasAuth = req.hasHeader("Authorization");
    
    // Get all headers as a map
    auto headers = req.headers();
    for (const auto& [name, value] : headers) {
        std::cout << name << ": " << value << std::endl;
    }
}
```

### Route Parameters

Access route parameters defined with the `:param` syntax in route paths:

```cpp
// Route defined as: app.get("/users/:id/posts/:postId", ...)
void handleRequest(const boson::Request& req, boson::Response& res) {
    // Get route parameters by name
    std::string userId = req.param("id");
    std::string postId = req.param("postId");
    
    // Get a parameter with a default value if not present
    std::string category = req.param("category", "general");
    
    // Get all parameters as a map
    auto params = req.params();
}
```

### Query Parameters

Access query string parameters from the URL:

```cpp
// Request URL: /search?q=boson&page=2&sort=desc
void handleRequest(const boson::Request& req, boson::Response& res) {
    // Get a specific query parameter
    std::string query = req.query("q");  // "boson"
    
    // Get a query parameter with a default value
    std::string page = req.query("page", "1");  // "2"
    std::string limit = req.query("limit", "10");  // "10" (default)
    
    // Get a query parameter as a specific type
    int pageNum = req.queryAs<int>("page", 1);  // 2
    
    // Check if a query parameter exists
    bool hasSort = req.hasQuery("sort");  // true
    
    // Get all query parameters as a map
    auto queries = req.queryParams();
    for (const auto& [key, value] : queries) {
        std::cout << key << ": " << value << std::endl;
    }
}
```

### Request Body

Access the request body in different formats:

```cpp
void handleRequest(const boson::Request& req, boson::Response& res) {
    // Get raw body as string
    std::string rawBody = req.body();
    
    // Parse JSON body
    nlohmann::json jsonBody = req.json();
    
    // Check if JSON body contains a key
    if (jsonBody.contains("username")) {
        std::string username = jsonBody["username"];
    }
    
    // Parse form data (application/x-www-form-urlencoded)
    auto formData = req.form();
    std::string name = formData["name"];
    
    // Access file uploads (multipart/form-data)
    auto files = req.files();
    for (const auto& file : files) {
        std::string fieldName = file.fieldName;
        std::string fileName = file.fileName;
        std::string contentType = file.contentType;
        size_t size = file.size;
        
        // Access file data
        const auto& data = file.data;
        
        // Or save the file to disk
        file.saveTo("/path/to/uploads/" + fileName);
    }
}
```

### Cookies

Access cookies from the request:

```cpp
void handleRequest(const boson::Request& req, boson::Response& res) {
    // Get a specific cookie
    std::string sessionId = req.cookie("sessionId");
    
    // Get a cookie with a default value
    std::string theme = req.cookie("theme", "light");
    
    // Check if a cookie exists
    bool hasConsent = req.hasCookie("cookieConsent");
    
    // Get all cookies as a map
    auto cookies = req.cookies();
}
```

## The Response Object

The `boson::Response` object represents the HTTP response that your server sends back to the client. It provides methods to set status codes, headers, and the response body.

### Setting Status Codes

```cpp
void handleRequest(const boson::Request& req, boson::Response& res) {
    // Set status code
    res.status(200);  // OK
    
    // Set status with predefined constants
    res.status(boson::StatusCode::CREATED);  // 201
    res.status(boson::StatusCode::NOT_FOUND);  // 404
    
    // Chainable API
    res.status(200).send("Success");
}
```

### Setting Headers

```cpp
void handleRequest(const boson::Request& req, boson::Response& res) {
    // Set a single header
    res.header("Content-Type", "application/json");
    
    // Set multiple headers
    res.header("Cache-Control", "no-cache")
       .header("X-Powered-By", "Boson");
    
    // Append to an existing header
    res.appendHeader("Set-Cookie", "theme=dark; Path=/");
    res.appendHeader("Set-Cookie", "sessionId=abc123; HttpOnly");
}
```

### Sending Responses

```cpp
void handleRequest(const boson::Request& req, boson::Response& res) {
    // Send a text response
    res.send("Hello, World!");
    
    // Send with a specific content type
    res.type("text/plain").send("Plain text response");
    
    // Send HTML
    res.send("<h1>Hello, Boson!</h1>");
    
    // Send JSON from a string
    res.type("application/json").send("{\"message\":\"Hello, World!\"}");
    
    // Send JSON from an object (using nlohmann::json)
    nlohmann::json jsonResponse = {
        {"message", "Hello, World!"},
        {"success", true},
        {"code", 200}
    };
    res.jsonObject(jsonResponse);
    
    // Send a JSON response directly
    res.jsonObject({
        {"message", "Hello, World!"},
        {"timestamp", "2025-04-15T12:30:00Z"}
    });
    
    // Send a file
    res.sendFile("/path/to/file.pdf");
    
    // Send a file with a specific filename
    res.sendFile("/path/to/file.pdf", "document.pdf");
    
    // Download a file (forces download rather than display)
    res.download("/path/to/file.pdf", "document.pdf");
    
    // Send with a specific status code (chaining)
    res.status(201).send("Resource created");
    
    // No content response
    res.status(204).send();
}
```

### Setting Cookies

```cpp
void handleRequest(const boson::Request& req, boson::Response& res) {
    // Set a basic cookie
    res.cookie("name", "value");
    
    // Set a cookie with options
    res.cookie("sessionId", "abc123", {
        {"maxAge", "3600"},      // 1 hour in seconds
        {"path", "/"},
        {"httpOnly", "true"},
        {"secure", "true"}
    });
    
    // Remove a cookie
    res.clearCookie("name");
    
    // Chain cookie operations
    res.cookie("theme", "dark")
       .cookie("lang", "en")
       .send("Cookies set");
}
```

### Redirects

```cpp
void handleRequest(const boson::Request& req, boson::Response& res) {
    // Simple redirect (302 Found)
    res.redirect("/new-location");
    
    // Redirect with specific status code
    res.redirect("/permanent-location", 301);  // Moved Permanently
    
    // Redirect to a different domain
    res.redirect("https://example.com/page");
}
```

### Response Streaming

For large responses, you can use streaming:

```cpp
void handleRequest(const boson::Request& req, boson::Response& res) {
    // Start a streaming response
    auto stream = res.stream();
    
    // Write data in chunks
    stream->write("Chunk 1");
    stream->write("Chunk 2");
    
    // End the stream
    stream->end();
}
```

### Compression

Enable response compression:

```cpp
void handleRequest(const boson::Request& req, boson::Response& res) {
    // Enable compression for this response
    res.compress(true);
    
    // Send a potentially large response
    res.sendFile("/path/to/large-file.txt");
}
```

## Best Practices

### Request Handling

1. **Validate input**: Always validate request parameters and body data
2. **Use proper error handling**: Catch exceptions when processing request data
3. **Handle missing values**: Provide default values for optional parameters
4. **Content negotiation**: Check Accept headers to serve the right format

```cpp
void handleRequest(const boson::Request& req, boson::Response& res) {
    try {
        // Validate required parameters
        if (!req.hasQuery("id")) {
            return res.status(400).jsonObject({
                {"error", "Missing required parameter 'id'"}
            });
        }
        
        // Parse and validate ID
        int id;
        try {
            id = std::stoi(req.query("id"));
        } catch (const std::exception&) {
            return res.status(400).jsonObject({
                {"error", "Invalid ID format"}
            });
        }
        
        // Content negotiation
        std::string accept = req.header("Accept", "application/json");
        if (accept.find("application/xml") != std::string::npos) {
            // Return XML response
            return res.type("application/xml").send("<response><message>Success</message></response>");
        }
        
        // Return JSON by default
        return res.jsonObject({
            {"message", "Success"}
        });
    } catch (const std::exception& e) {
        return res.status(500).jsonObject({
            {"error", "Server error"},
            {"message", e.what()}
        });
    }
}
```

### Response Best Practices

1. **Set appropriate status codes**: Use the right HTTP status code for each response
2. **Set correct content types**: Always set the Content-Type header properly
3. **Security headers**: Include security-related headers (e.g., Content-Security-Policy)
4. **Consistency**: Maintain a consistent response format

```cpp
void handleRequest(const boson::Request& req, boson::Response& res) {
    // Set security headers
    res.header("X-Content-Type-Options", "nosniff")
       .header("X-Frame-Options", "DENY")
       .header("Content-Security-Policy", "default-src 'self'");
    
    // Use consistent response format
    res.jsonObject({
        {"success", true},
        {"data", {
            {"id", 123},
            {"name", "Example"}
        }},
        {"meta", {
            {"version", "1.0"},
            {"timestamp", "2025-04-15T12:30:00Z"}
        }}
    });
}
```
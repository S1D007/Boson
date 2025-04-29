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

### Multiple Query Parameters with Same Name

Handle query parameters that appear multiple times:

```cpp
// Request URL: /filter?tag=cpp&tag=web&tag=api
void handleRequest(const boson::Request& req, boson::Response& res) {
    // Get all values for a query parameter
    std::vector<std::string> tags = req.queryArray("tag");  // ["cpp", "web", "api"]
    
    // Convert to a string for display
    std::string tagList;
    for (size_t i = 0; i < tags.size(); ++i) {
        tagList += tags[i];
        if (i < tags.size() - 1) {
            tagList += ", ";
        }
    }
    
    res.send("Selected tags: " + tagList);
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

### Working with JSON

Detailed examples of working with JSON request bodies:

```cpp
void handleUserRequest(const boson::Request& req, boson::Response& res) {
    try {
        // Parse the JSON body
        auto userData = req.json();
        
        // Basic validation
        if (!userData.contains("name") || !userData.contains("email")) {
            return res.status(400).jsonObject({
                {"error", "Missing required fields"},
                {"requiredFields", {"name", "email"}}
            });
        }
        
        // Type checking
        if (!userData["name"].is_string() || !userData["email"].is_string()) {
            return res.status(400).jsonObject({
                {"error", "Invalid field types"},
                {"fields", {
                    {"name", "string"},
                    {"email", "string"}
                }}
            });
        }
        
        // Extract and use the data
        std::string name = userData["name"].get<std::string>();
        std::string email = userData["email"].get<std::string>();
        
        // Optional fields with default values
        int age = 0;
        if (userData.contains("age") && userData["age"].is_number()) {
            age = userData["age"].get<int>();
        }
        
        // Nested objects
        std::string city, country;
        if (userData.contains("address") && userData["address"].is_object()) {
            auto& address = userData["address"];
            if (address.contains("city") && address["city"].is_string()) {
                city = address["city"].get<std::string>();
            }
            if (address.contains("country") && address["country"].is_string()) {
                country = address["country"].get<std::string>();
            }
        }
        
        // Arrays
        std::vector<std::string> tags;
        if (userData.contains("tags") && userData["tags"].is_array()) {
            for (const auto& tag : userData["tags"]) {
                if (tag.is_string()) {
                    tags.push_back(tag.get<std::string>());
                }
            }
        }
        
        // Process the data (here just echoing it back)
        return res.status(200).jsonObject({
            {"message", "User data received"},
            {"user", {
                {"name", name},
                {"email", email},
                {"age", age},
                {"address", {
                    {"city", city},
                    {"country", country}
                }},
                {"tags", tags}
            }}
        });
    }
    catch (const nlohmann::json::exception& e) {
        return res.status(400).jsonObject({
            {"error", "Invalid JSON format"},
            {"message", e.what()}
        });
    }
    catch (const std::exception& e) {
        return res.status(500).jsonObject({
            {"error", "Server error"},
            {"message", e.what()}
        });
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

### Session Data

If sessions are enabled with middleware, you can access session data:

```cpp
void handleSessionRequest(const boson::Request& req, boson::Response& res) {
    // Get session data
    auto session = req.session();
    
    // Check if user is authenticated
    if (session.contains("userId")) {
        std::string userId = session["userId"].get<std::string>();
        res.send("Hello, User " + userId);
    } else {
        res.redirect("/login");
    }
    
    // Update session data
    session["lastAccess"] = std::time(nullptr);
    session["pageVisits"] = session.value("pageVisits", 0) + 1;
    req.saveSession(session);
}
```

### Custom Request Properties

Store and retrieve custom data during request processing:

```cpp
// In a middleware
void authMiddleware(const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    // Extract and verify token
    std::string token = req.header("Authorization", "");
    
    if (!token.empty()) {
        // Store user information in the request for later use
        auto user = authenticateToken(token);  // Your authentication function
        req.set("user", user);
    }
    
    next();
}

// In a route handler
void userProfileHandler(const boson::Request& req, boson::Response& res) {
    // Check if user is authenticated
    if (!req.has("user")) {
        return res.status(401).jsonObject({{"error", "Unauthorized"}});
    }
    
    // Get user data
    auto user = req.get<User>("user");
    
    // Use the user data
    res.jsonObject({
        {"profile", {
            {"id", user.id},
            {"name", user.name},
            {"email", user.email}
        }}
    });
}
```

### Processing Raw Request Body

For cases where automatic body parsing isn't sufficient:

```cpp
void handleRawRequest(const boson::Request& req, boson::Response& res) {
    // Get raw body as string
    std::string rawBody = req.body();
    
    // Process custom format (for example, XML)
    if (req.contentType().find("application/xml") != std::string::npos) {
        // Custom XML parsing (using hypothetical XML parser)
        auto xml = XMLParser::parse(rawBody);
        std::string username = xml.getElement("user").getAttribute("name");
        
        res.send("Hello, " + username);
    } else {
        res.status(415).send("Unsupported Media Type");
    }
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

### Common Status Codes

Here are the most commonly used HTTP status codes and when to use them:

```cpp
// Success responses
res.status(200).send("OK");                        // Standard success
res.status(201).send("Created");                   // Resource created
res.status(202).send("Accepted");                  // Request accepted for processing
res.status(204).send("");                          // No content to send back

// Redirection responses
res.status(301).redirect("/new-permanent-url");    // Moved permanently
res.status(302).redirect("/temporary-url");        // Found (temporary redirect)
res.status(304).send("");                          // Not modified (use cached version)

// Client error responses
res.status(400).send("Bad Request");               // Invalid request
res.status(401).send("Unauthorized");              // Authentication required
res.status(403).send("Forbidden");                 // Authenticated but not authorized
res.status(404).send("Not Found");                 // Resource not found
res.status(405).send("Method Not Allowed");        // HTTP method not allowed for resource
res.status(409).send("Conflict");                  // Request conflicts with server state
res.status(422).send("Unprocessable Entity");      // Validation errors

// Server error responses
res.status(500).send("Internal Server Error");     // Generic server error
res.status(501).send("Not Implemented");           // Endpoint not implemented
res.status(503).send("Service Unavailable");       // Server temporarily unavailable
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

### Standard HTTP Headers

Examples of common HTTP headers and their usage:

```cpp
// Content-related headers
res.header("Content-Type", "application/json; charset=utf-8");
res.header("Content-Length", std::to_string(bodySize));
res.header("Content-Encoding", "gzip");
res.header("Content-Language", "en-US");

// Caching headers
res.header("Cache-Control", "max-age=3600, must-revalidate");
res.header("ETag", "\"123456789\"");
res.header("Last-Modified", "Wed, 15 Apr 2025 12:30:00 GMT");
res.header("Expires", "Wed, 15 Apr 2025 13:30:00 GMT");

// CORS headers
res.header("Access-Control-Allow-Origin", "*");
res.header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");
res.header("Access-Control-Allow-Headers", "Content-Type, Authorization");
res.header("Access-Control-Max-Age", "86400");  // 24 hours

// Security headers
res.header("Strict-Transport-Security", "max-age=31536000; includeSubDomains");
res.header("X-Content-Type-Options", "nosniff");
res.header("X-Frame-Options", "DENY");
res.header("Content-Security-Policy", "default-src 'self'");
res.header("X-XSS-Protection", "1; mode=block");
res.header("Referrer-Policy", "no-referrer-when-downgrade");

// Custom headers (prefixed with X-)
res.header("X-Request-ID", requestId);
res.header("X-API-Version", "1.0");
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

### Cookie Security Options

```cpp
void setCookieWithSecurity(const boson::Request& req, boson::Response& res) {
    // Secure cookie with all security options
    res.cookie("sessionId", generateSessionId(), {
        {"maxAge", "86400"},        // 24 hours in seconds
        {"expires", "Wed, 15 Apr 2025 12:30:00 GMT"}, // Explicit expiration
        {"path", "/"},              // Cookie path
        {"domain", "example.com"},  // Cookie domain
        {"secure", "true"},         // HTTPS only
        {"httpOnly", "true"},       // Not accessible via JavaScript
        {"sameSite", "strict"}      // Strict same-site enforcement
    });
    
    // Session cookie (expires when browser closes)
    res.cookie("preference", "darkMode", {
        {"path", "/"},
        {"sameSite", "lax"}         // Lax same-site enforcement
    });
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

### Advanced Redirect Scenarios

```cpp
void handleRedirects(const boson::Request& req, boson::Response& res) {
    // Redirect based on user agent
    std::string userAgent = req.header("User-Agent");
    if (userAgent.find("Mobile") != std::string::npos) {
        res.redirect("/mobile");
        return;
    }
    
    // Redirect with query parameters
    std::string redirectUrl = "/dashboard?";
    redirectUrl += "user=" + encodeURIComponent(req.query("user"));
    redirectUrl += "&ref=login";
    res.redirect(redirectUrl);
    
    // Redirect to the previous page (if available)
    std::string referer = req.header("Referer", "");
    if (!referer.empty()) {
        res.redirect(referer);
    } else {
        res.redirect("/default");
    }
    
    // PRG Pattern (Post/Redirect/Get)
    // After form submission (POST), redirect to prevent duplicate submissions
    if (req.method() == "POST") {
        // Process form...
        res.redirect("/success?id=123", 303);  // 303 See Other
    }
}
```

### Response Streaming

For large responses, you can use streaming to send data in chunks, which helps with:
- Managing memory efficiently when handling large files
- Delivering content to clients faster by sending available data immediately
- Supporting real-time data that's generated over time

#### Basic Streaming Example

```cpp
void handleRequest(const boson::Request& req, boson::Response& res) {
    // Start a streaming response
    res.header("Content-Type", "text/plain");
    res.stream(true);
    
    // Write data in chunks
    res.write("Chunk 1");
    res.write("Chunk 2");
    
    // End the stream
    res.end();
}
```

#### Streaming Files

Boson provides several convenient ways to stream files:

##### Automatic Streaming for Large Files

By default, Boson automatically uses streaming for files larger than 1MB:

```cpp
void sendLargeFile(const boson::Request& req, boson::Response& res) {
    // Will automatically use streaming for files larger than 1MB
    res.sendFile("/path/to/large-file.dat");
}
```

##### Explicit File Streaming

You can explicitly choose to stream any file regardless of size:

```cpp
void explicitStreamFile(const boson::Request& req, boson::Response& res) {
    // Explicitly use streaming for any file
    std::map<std::string, std::any> options = {
        {"stream", true}
    };
    res.streamFile("/path/to/file.txt", options);
}
```

##### Customizing Streaming Behavior

Customize chunk size for fine-tuning performance:

```cpp
void customizedStreaming(const boson::Request& req, boson::Response& res) {
    // Set a custom chunk size (in bytes)
    std::map<std::string, std::any> options = {
        {"stream", true},
        {"chunkSize", static_cast<size_t>(4096)}  // 4KB chunks
    };
    res.streamFile("/path/to/file.dat", options);
}
```

#### File Download with Streaming

For file downloads, you can combine streaming with download headers:

```cpp
void streamedDownload(const boson::Request& req, boson::Response& res) {
    // Streaming file download with custom filename
    std::map<std::string, std::any> options = {
        {"stream", true}
    };
    res.download("/path/to/report.pdf", "monthly-report-april-2025.pdf", options);
}
```

#### Streaming Dynamically Generated Content

Stream data as it's being generated:

```cpp
void streamGeneratedData(const boson::Request& req, boson::Response& res) {
    // For data that's generated on the fly
    res.header("Content-Type", "application/json");
    res.stream(true);
    
    // Start JSON array
    res.write("[");
    
    // Generate and stream items one by one
    for (int i = 0; i < 1000; i++) {
        // Generate an item
        nlohmann::json item = {
            {"id", i},
            {"name", "Item " + std::to_string(i)},
            {"timestamp", std::time(nullptr)}
        };
        
        // Write item, with comma if it's not the last one
        res.write(item.dump() + (i < 999 ? "," : ""));
        
        // In a real app, you might add a delay or check for client disconnection
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // End JSON array and complete the response
    res.write("]");
    res.end();
}
```

#### Streaming Large Files or Data with Low Memory Usage

When handling very large files, you can stream them with minimal memory overhead:

```cpp
void streamLargeFile(const boson::Request& req, boson::Response& res) {
    // Set appropriate headers for streaming
    res.header("Content-Type", "application/octet-stream");
    res.stream(true);
    
    // Open a file (using standard C++ file handling)
    std::ifstream file("/path/to/very-large-file.bin", std::ios::binary);
    if (!file) {
        res.status(404).send("File not found");
        return;
    }
    
    // Read and stream the file in chunks
    const size_t bufferSize = 8192;  // 8KB chunks
    char buffer[bufferSize];
    
    while (file.read(buffer, bufferSize)) {
        res.write(std::string(buffer, file.gcount()));
    }
    
    // Write any remaining data
    if (file.gcount() > 0) {
        res.write(std::string(buffer, file.gcount()));
    }
    
    // Close the stream
    res.end();
}
```

#### Behind the Scenes: How Streaming Works

When you use streaming in Boson:

1. Boson sets `Transfer-Encoding: chunked` in the HTTP headers
2. Each chunk is sent in the HTTP chunked encoding format:
   - Size of the chunk in hexadecimal format
   - CRLF (carriage return + line feed)
   - The chunk data itself
   - CRLF
3. The final chunk is a zero-sized chunk followed by optional trailers and a final CRLF

#### When to Use Streaming

Use streaming when:

- Sending large files (>1MB) to avoid loading the entire file into memory
- Delivering real-time data as it becomes available
- Building APIs for long-running processes with progressive results
- Creating server-sent events (SSE) endpoints
- Implementing logging or monitoring endpoints that continuously deliver data

#### Performance Considerations

When using streaming:

- Choose an appropriate chunk size for your use case (typically 4-16KB)
- For very large files (>100MB), always use streaming to avoid memory issues
- Consider adding progress indication for long-running streams
- Monitor client connections and stop processing if the client disconnects
- Remember that streaming ties up a connection for the duration of the stream

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

### Response Templates

Using templates for HTML responses:

```cpp
// Simple templating function (in a real app, use a proper template engine)
std::string renderTemplate(const std::string& templateStr, 
                          const std::unordered_map<std::string, std::string>& vars) {
    std::string result = templateStr;
    for (const auto& [key, value] : vars) {
        std::string placeholder = "{{" + key + "}}";
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), value);
            pos += value.length();
        }
    }
    return result;
}

void handleTemplatedResponse(const boson::Request& req, boson::Response& res) {
    // Template string (in a real app, load from file)
    std::string templateStr = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <title>{{title}}</title>
        </head>
        <body>
            <h1>{{heading}}</h1>
            <p>Hello, {{name}}!</p>
        </body>
        </html>
    )";
    
    // Render template with variables
    std::string html = renderTemplate(templateStr, {
        {"title", "Welcome Page"},
        {"heading", "Welcome to Boson"},
        {"name", req.query("name", "Guest")}
    });
    
    // Send the rendered HTML
    res.type("text/html").send(html);
}
```

## Advanced Request-Response Patterns

### Content Negotiation

Choose response format based on Accept header:

```cpp
void contentNegotiation(const boson::Request& req, boson::Response& res) {
    // Data to respond with
    std::string name = "John Doe";
    int age = 30;
    std::vector<std::string> hobbies = {"reading", "coding", "hiking"};
    
    // Get the Accept header
    std::string accept = req.header("Accept", "*/*");
    
    // Respond with appropriate format
    if (accept.find("application/json") != std::string::npos) {
        // JSON response
        return res.jsonObject({
            {"name", name},
            {"age", age},
            {"hobbies", hobbies}
        });
    }
    else if (accept.find("application/xml") != std::string::npos) {
        // XML response (manually constructed)
        std::stringstream xml;
        xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        xml << "<person>\n";
        xml << "  <name>" << name << "</name>\n";
        xml << "  <age>" << age << "</age>\n";
        xml << "  <hobbies>\n";
        for (const auto& hobby : hobbies) {
            xml << "    <hobby>" << hobby << "</hobby>\n";
        }
        xml << "  </hobbies>\n";
        xml << "</person>";
        
        return res.type("application/xml").send(xml.str());
    }
    else if (accept.find("text/html") != std::string::npos) {
        // HTML response
        std::stringstream html;
        html << "<!DOCTYPE html>\n";
        html << "<html>\n";
        html << "<head><title>Person Information</title></head>\n";
        html << "<body>\n";
        html << "  <h1>" << name << "</h1>\n";
        html << "  <p>Age: " << age << "</p>\n";
        html << "  <h2>Hobbies:</h2>\n";
        html << "  <ul>\n";
        for (const auto& hobby : hobbies) {
            html << "    <li>" << hobby << "</li>\n";
        }
        html << "  </ul>\n";
        html << "</body>\n";
        html << "</html>";
        
        return res.type("text/html").send(html.str());
    }
    else {
        // Plain text as fallback
        std::stringstream text;
        text << "Name: " << name << "\n";
        text << "Age: " << age << "\n";
        text << "Hobbies: ";
        for (size_t i = 0; i < hobbies.size(); ++i) {
            text << hobbies[i];
            if (i < hobbies.size() - 1) {
                text << ", ";
            }
        }
        
        return res.type("text/plain").send(text.str());
    }
}
```

### Conditional Responses

Send responses based on If-Modified-Since or ETag:

```cpp
void conditionalResponse(const boson::Request& req, boson::Response& res) {
    // Resource information
    std::string content = "This is the resource content";
    std::string etag = "\"abc123\"";  // Calculate based on content
    std::time_t lastModified = std::time(nullptr) - 3600;  // 1 hour ago
    char lastModifiedStr[50];
    std::strftime(lastModifiedStr, sizeof(lastModifiedStr), "%a, %d %b %Y %H:%M:%S GMT", 
                 std::gmtime(&lastModified));
    
    // Set ETag and Last-Modified headers
    res.header("ETag", etag);
    res.header("Last-Modified", lastModifiedStr);
    
    // Check If-None-Match (ETag)
    std::string ifNoneMatch = req.header("If-None-Match", "");
    if (ifNoneMatch == etag) {
        return res.status(304).send();  // Not Modified
    }
    
    // Check If-Modified-Since
    std::string ifModifiedSince = req.header("If-Modified-Since", "");
    if (!ifModifiedSince.empty()) {
        std::tm tm = {};
        std::istringstream ss(ifModifiedSince);
        ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
        std::time_t clientTime = std::mktime(&tm);
        
        if (clientTime >= lastModified) {
            return res.status(304).send();  // Not Modified
        }
    }
    
    // Send the full response if conditions aren't met
    res.send(content);
}
```

### Handling CORS Preflight Requests

```cpp
void handleCors(const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
    // Set CORS headers
    res.header("Access-Control-Allow-Origin", "*");
    res.header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    res.header("Access-Control-Max-Age", "86400");  // 24 hours
    
    // Handle OPTIONS method (preflight request)
    if (req.method() == "OPTIONS") {
        return res.status(204).send();  // No content needed for preflight
    }
    
    // Continue for non-OPTIONS methods
    next();
}
```

### Handling File Uploads

```cpp
void fileUploadHandler(const boson::Request& req, boson::Response& res) {
    auto files = req.files();
    
    if (files.empty()) {
        return res.status(400).jsonObject({{"error", "No files uploaded"}});
    }
    
    std::vector<nlohmann::json> uploadedFiles;
    
    for (const auto& file : files) {
        // Check file type
        if (file.contentType.find("image/") != 0) {
            return res.status(400).jsonObject({
                {"error", "Only image files are allowed"},
                {"uploaded", file.contentType}
            });
        }
        
        // Check file size
        if (file.size > 5 * 1024 * 1024) {  // 5MB
            return res.status(400).jsonObject({
                {"error", "File too large"},
                {"maxSize", "5MB"},
                {"uploaded", file.size / 1024 / 1024.0}
            });
        }
        
        // Generate unique filename
        std::string uniqueName = generateUniqueId() + "_" + file.fileName;
        std::string savePath = "/path/to/uploads/" + uniqueName;
        
        // Save file
        try {
            file.saveTo(savePath);
            
            // Add to list of uploaded files
            uploadedFiles.push_back({
                {"originalName", file.fileName},
                {"savedName", uniqueName},
                {"size", file.size},
                {"type", file.contentType},
                {"url", "/uploads/" + uniqueName}
            });
        }
        catch (const std::exception& e) {
            return res.status(500).jsonObject({
                {"error", "Failed to save file"},
                {"message", e.what()}
            });
        }
    }
    
    // Return information about uploaded files
    res.status(201).jsonObject({
        {"message", "Files uploaded successfully"},
        {"count", uploadedFiles.size()},
        {"files", uploadedFiles}
    });
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

### Structured Error Responses

Create consistent error responses across your API:

```cpp
void sendErrorResponse(boson::Response& res, int statusCode, 
                       const std::string& errorType, 
                       const std::string& message,
                       const nlohmann::json& details = nullptr) {
    nlohmann::json errorResponse = {
        {"error", {
            {"type", errorType},
            {"message", message},
            {"status", statusCode}
        }}
    };
    
    if (details != nullptr) {
        errorResponse["error"]["details"] = details;
    }
    
    res.status(statusCode).jsonObject(errorResponse);
}

// Usage examples
void handleRequest(const boson::Request& req, boson::Response& res) {
    try {
        if (!req.hasQuery("id")) {
            return sendErrorResponse(res, 400, "missing_parameter", "Missing required parameter 'id'");
        }
        
        std::string id = req.query("id");
        
        if (!isValidId(id)) {
            return sendErrorResponse(res, 400, "invalid_parameter", "Invalid ID format", {
                {"parameter", "id"},
                {"provided", id},
                {"requirement", "Must be a numeric string"}
            });
        }
        
        // Resource not found example
        if (id == "999") {
            return sendErrorResponse(res, 404, "resource_not_found", "User not found", {
                {"id", id}
            });
        }
        
        // Access denied example
        if (id == "123" && !isAuthorized(req)) {
            return sendErrorResponse(res, 403, "access_denied", "You don't have permission to view this user");
        }
        
        // Success response
        res.jsonObject({
            {"id", id},
            {"name", "Test User"}
        });
    }
    catch (const DatabaseException& e) {
        return sendErrorResponse(res, 500, "database_error", "Database operation failed", {
            {"details", e.what()}
        });
    }
    catch (const std::exception& e) {
        return sendErrorResponse(res, 500, "server_error", "An unexpected error occurred");
    }
}
```

### Optimizing Response Handling

Tips for optimizing response performance:

```cpp
void optimizedResponse(const boson::Request& req, boson::Response& res) {
    // 1. Set appropriate cache headers for static content
    if (req.path().starts_with("/static/")) {
        res.header("Cache-Control", "public, max-age=86400");  // 24 hours
        res.header("Expires", getExpiryDateString(86400));
    }
    
    // 2. Use compression for text-based responses
    if (req.header("Accept-Encoding").find("gzip") != std::string::npos) {
        res.header("Content-Encoding", "gzip");
        res.compress(true);
    }
    
    // 3. Minimize payload size for JSON responses
    if (req.query("fields") == "minimal") {
        // Return only essential fields
        return res.jsonObject({
            {"id", 123},
            {"name", "Product Name"}
        });
    } else {
        // Return all fields
        return res.jsonObject({
            {"id", 123},
            {"name", "Product Name"},
            {"description", "Detailed description..."},
            {"price", 99.99},
            {"inventory", 50},
            {"categories", {"Electronics", "Gadgets"}},
            {"attributes", {
                {"color", "Black"},
                {"weight", "250g"},
                {"dimensions", "10x5x2 cm"}
            }},
            {"images", {
                "https://example.com/products/123/1.jpg",
                "https://example.com/products/123/2.jpg"
            }}
        });
    }
}
```

### Response Testing Checklist

When implementing response handlers, consider testing these aspects:

1. **Status codes**: Verify the correct status code is returned for various scenarios
2. **Headers**: Ensure all required headers are set correctly
3. **Body format**: Validate the response body structure (JSON schema, etc.)
4. **Error handling**: Test various error conditions for proper responses
5. **Edge cases**: Handle empty results, large payloads, special characters, etc.
6. **Performance**: Measure response time for typical and extreme loads
7. **Caching**: Verify that caching headers work as expected
8. **Content negotiation**: Test different Accept headers
9. **Character encoding**: Ensure proper handling of non-ASCII characters
10. **Security**: Validate that sensitive information isn't exposed

## Conclusion

Understanding Boson's Request and Response objects thoroughly is essential for building efficient and reliable web applications. These objects provide a comprehensive API for handling HTTP interactions in a type-safe, flexible manner.

In the next section, we'll explore [Controllers](./controllers), which help organize route handlers and provide a more structured approach to managing your application's endpoints.
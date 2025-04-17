#include <boson/boson.hpp>
#include <iostream>
#include <filesystem>
#include <string>

int main() {
    try {
        boson::initialize();
        
        boson::Server app;
        
        app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
            std::cout << "[" << req.method() << "] " << req.path() << std::endl;
            next();
        });
        
        std::filesystem::path publicPath = std::filesystem::current_path() / "public";
        
        if (!std::filesystem::exists(publicPath)) {
            std::filesystem::create_directories(publicPath);
            std::cout << "Created public directory: " << publicPath << std::endl;
        }
        
        std::unordered_map<std::string, std::string> options = {
            {"cacheControl", "max-age=3600"} 
        };
        
        app.use(boson::StaticFiles::create(publicPath.string(), "/", options));
        std::cout << "Serving static files from: " << publicPath << std::endl;
        
        app.get("/", [](const boson::Request& req, boson::Response& res) {
            res.header("Content-Type", "text/html");
            res.send(R"(
                <!DOCTYPE html>
                <html>
                <head>
                    <title>Boson Static Files Example</title>
                    <link rel="stylesheet" href="/styles.css">
                </head>
                <body>
                    <h1>Boson Static Files Example</h1>
                    <p>This is an example of serving static files with Boson Framework.</p>
                    <p>If you see styled text, the CSS file was loaded correctly!</p>
                    <div class="info-box">
                        <h2>How it works</h2>
                        <p>The StaticFiles middleware serves files from the 'public' directory.</p>
                    </div>
                    <script src="/app.js"></script>
                </body>
                </html>
            )");
        });
        
        unsigned int port = 3000;
        std::string host = "127.0.0.1";
        app.configure(port, host);
        
        std::cout << "Server running at http://" << host << ":" << port << std::endl;
        std::cout << "Try creating files in the 'public' directory to serve them statically." << std::endl;
        
        return app.listen();
    }
    catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
}
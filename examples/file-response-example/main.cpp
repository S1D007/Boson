#include <boson/boson.hpp>
#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <random>
#include <chrono>
#include <thread>

std::string generateRandomData(size_t sizeInMB) {
    const size_t sizeInBytes = sizeInMB * 1024 * 1024;
    std::string data;
    data.reserve(sizeInBytes);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(32, 126);
    
    for (size_t i = 0; i < sizeInBytes; i++) {
        data.push_back(static_cast<char>(dist(gen)));
    }
    
    return data;
}

bool createTestFiles(const std::filesystem::path& directory) {
    try {
        // Create a small text file
        std::ofstream smallText(directory / "small.txt");
        smallText << "This is a small text file created for the file response example.\n";
        smallText << "It demonstrates sending small files directly in the response.\n";
        smallText.close();
        
        // Create a small PDF file
        std::ofstream pdfFile(directory / "sample.pdf");
        pdfFile << "%PDF-1.7\n1 0 obj<</Type/Catalog/Pages 2 0 R>>endobj\n";
        pdfFile << "2 0 obj<</Type/Pages/Kids[3 0 R]/Count 1>>endobj\n";
        pdfFile << "3 0 obj<</Type/Page/MediaBox[0 0 612 792]/Parent 2 0 R/Resources<<>>>>\n";
        pdfFile << "endobj\nxref\n0 4\n0000000000 65535 f\n0000000010 00000 n\n";
        pdfFile << "0000000053 00000 n\n0000000102 00000 n\ntrailer<</Size 4/Root 1 0 R>>\n";
        pdfFile << "startxref\n183\n%%EOF\n";
        pdfFile.close();
        
        // Create a JSON file
        std::ofstream jsonFile(directory / "data.json");
        jsonFile << "{\n";
        jsonFile << "  \"name\": \"Boson Framework\",\n";
        jsonFile << "  \"version\": \"1.0.0\",\n";
        jsonFile << "  \"features\": [\n";
        jsonFile << "    \"HTTP server\",\n";
        jsonFile << "    \"Routing\",\n";
        jsonFile << "    \"Middleware\",\n";
        jsonFile << "    \"File responses\",\n";
        jsonFile << "    \"JSON support\"\n";
        jsonFile << "  ],\n";
        jsonFile << "  \"performance\": {\n";
        jsonFile << "    \"requests_per_second\": 50000,\n";
        jsonFile << "    \"average_latency_ms\": 2.5\n";
        jsonFile << "  }\n";
        jsonFile << "}\n";
        jsonFile.close();
        
        // Create a 5MB file to demonstrate streaming
        std::ofstream largeFile(directory / "large_file.bin", std::ios::binary);
        std::string randomData = generateRandomData(5); // 5MB
        largeFile.write(randomData.c_str(), randomData.size());
        largeFile.close();
        
        // Create a 20MB file to demonstrate auto-streaming
        std::ofstream veryLargeFile(directory / "very_large_file.bin", std::ios::binary);
        std::string moreRandomData = generateRandomData(20);
        veryLargeFile.write(moreRandomData.c_str(), moreRandomData.size());
        veryLargeFile.close();
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error creating test files: " << e.what() << std::endl;
        return false;
    }
}

int main() {
    try {
        boson::initialize();
        
        boson::Server app;
        
        app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
            std::cout << "[" << req.method() << "] " << req.path() << std::endl;
            next();
        });
        
        std::filesystem::path publicPath = std::filesystem::current_path() / "public";
        std::filesystem::path filesPath = publicPath / "files";
        
        if (!std::filesystem::exists(filesPath)) {
            std::filesystem::create_directories(filesPath);
            std::cout << "Created files directory: " << filesPath << std::endl;
            
            if (createTestFiles(filesPath)) {
                std::cout << "Test files created successfully" << std::endl;
            }
        }
        
        app.use(boson::StaticFiles::create(publicPath.string(), "/", {{"cacheControl", "max-age=3600"}}));
        
        app.get("/", [](const boson::Request& req, boson::Response& res) {
            res.header("Content-Type", "text/html");
            res.send(R"(
                <!DOCTYPE html>
                <html>
                <head>
                    <title>Boson File Response Demo</title>
                    <style>
                        body {
                            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                            line-height: 1.6;
                            color: #333;
                            max-width: 800px;
                            margin: 0 auto;
                            padding: 20px;
                        }
                        h1 {
                            color: #2c3e50;
                            border-bottom: 2px solid #3498db;
                            padding-bottom: 10px;
                        }
                        .section {
                            background-color: #f8f9fa;
                            border-radius: 5px;
                            padding: 20px;
                            margin-bottom: 20px;
                        }
                        h2 {
                            color: #3498db;
                            margin-top: 0;
                        }
                        ul {
                            padding-left: 20px;
                        }
                        a {
                            color: #2980b9;
                            text-decoration: none;
                        }
                        a:hover {
                            text-decoration: underline;
                        }
                        .code {
                            background-color: #f1f1f1;
                            padding: 2px 5px;
                            border-radius: 3px;
                            font-family: monospace;
                        }
                    </style>
                </head>
                <body>
                    <h1>Boson File Response Demo</h1>
                    
                    <div class="section">
                        <h2>Regular File Responses</h2>
                        <p>These examples demonstrate sending files with proper MIME type detection:</p>
                        <ul>
                            <li><a href="/files/text">Send text file</a> (<span class="code">sendFile</span>)</li>
                            <li><a href="/files/json">Send JSON file</a> (<span class="code">sendFile</span> with JSON content type)</li>
                            <li><a href="/files/pdf">Send PDF file</a> (<span class="code">sendFile</span> with PDF content type)</li>
                        </ul>
                    </div>
                    
                    <div class="section">
                        <h2>File Downloads</h2>
                        <p>These examples demonstrate forcing file download with Content-Disposition header:</p>
                        <ul>
                            <li><a href="/download/text">Download text file</a> (<span class="code">download</span>)</li>
                            <li><a href="/download/json">Download JSON file</a> (<span class="code">download</span>)</li>
                            <li><a href="/download/pdf">Download PDF with custom name</a> (<span class="code">download</span> with custom filename)</li>
                        </ul>
                    </div>
                    
                    <div class="section">
                        <h2>File Streaming</h2>
                        <p>These examples demonstrate streaming large files with chunked transfer encoding:</p>
                        <ul>
                            <li><a href="/stream/large">Stream 5MB file</a> (<span class="code">streamFile</span> explicit)</li>
                            <li><a href="/stream/auto">Stream 20MB file</a> (auto-detection of large files)</li>
                            <li><a href="/stream/custom">Stream with custom chunk size</a> (2KB chunks)</li>
                        </ul>
                    </div>
                    
                    <div class="section">
                        <h2>Advanced Features</h2>
                        <p>These examples demonstrate additional features of file responses:</p>
                        <ul>
                            <li><a href="/advanced/etag">ETag support</a> (caching with If-None-Match)</li>
                            <li><a href="/advanced/modified">Last-Modified support</a> (caching with If-Modified-Since)</li>
                            <li><a href="/advanced/cache">Custom Cache-Control</a> (configured caching behavior)</li>
                            <li><a href="/advanced/range">Range requests</a> (partial content for byte ranges)</li>
                        </ul>
                    </div>
                </body>
                </html>
            )");
        });
        
        app.get("/files/text", [&filesPath](const boson::Request& req, boson::Response& res) {
            res.sendFile((filesPath / "small.txt").string());
        });
        
        app.get("/files/json", [&filesPath](const boson::Request& req, boson::Response& res) {
            res.sendFile((filesPath / "data.json").string());
        });
        
        app.get("/files/pdf", [&filesPath](const boson::Request& req, boson::Response& res) {
            res.sendFile((filesPath / "sample.pdf").string());
        });
        
        app.get("/download/text", [&filesPath](const boson::Request& req, boson::Response& res) {
            res.download((filesPath / "small.txt").string());
        });
        
        app.get("/download/json", [&filesPath](const boson::Request& req, boson::Response& res) {
            res.download((filesPath / "data.json").string());
        });
        
        app.get("/download/pdf", [&filesPath](const boson::Request& req, boson::Response& res) {
            res.download((filesPath / "sample.pdf").string(), "boson-framework-documentation.pdf");
        });
        
        app.get("/stream/large", [&filesPath](const boson::Request& req, boson::Response& res) {
            std::map<std::string, std::any> options = {
                {"stream", true} 
            };
            res.streamFile((filesPath / "large_file.bin").string(), options);
        });
        
        app.get("/stream/auto", [&filesPath](const boson::Request& req, boson::Response& res) {
            res.sendFile((filesPath / "very_large_file.bin").string());
        });
        
        app.get("/stream/custom", [&filesPath](const boson::Request& req, boson::Response& res) {
            std::map<std::string, std::any> options = {
                {"stream", true},
                {"chunkSize", static_cast<size_t>(2048)}
            };
            res.streamFile((filesPath / "large_file.bin").string(), options);
        });
        
        app.get("/advanced/etag", [&filesPath](const boson::Request& req, boson::Response& res) {
            std::map<std::string, std::any> options = {
                {"etag", "\"custom-etag-value-12345\""}
            };
            
            std::string ifNoneMatch = req.header("If-None-Match");
            if (ifNoneMatch == "\"custom-etag-value-12345\"") {
                res.status(304).send("");
            }
            
            res.sendFile((filesPath / "data.json").string(), options);
        });
        
        app.get("/advanced/modified", [&filesPath](const boson::Request& req, boson::Response& res) {
            std::string ifModifiedSince = req.header("If-Modified-Since");
            if (!ifModifiedSince.empty()) {
                res.status(304).send("");
                return;
            }
            
            res.sendFile((filesPath / "small.txt").string());
        });
        
        app.get("/advanced/cache", [&filesPath](const boson::Request& req, boson::Response& res) {
            std::map<std::string, std::any> options = {
                {"cacheControl", "public, max-age=86400, must-revalidate"}
            };
            res.sendFile((filesPath / "data.json").string(), options);
        });
        
        app.get("/advanced/range", [&filesPath](const boson::Request& req, boson::Response& res) {
            std::string rangeHeader = req.header("Range");
            std::filesystem::path filePath = filesPath / "large_file.bin";
            
            if (rangeHeader.empty() || !std::filesystem::exists(filePath)) {
                res.sendFile(filePath.string());
            }
            
            // Parse range header (e.g., "bytes=0-1023")
            size_t startPos = 0, endPos = 0;
            if (rangeHeader.substr(0, 6) == "bytes=") {
                std::string rangeValue = rangeHeader.substr(6);
                size_t dashPos = rangeValue.find('-');
                if (dashPos != std::string::npos) {
                    startPos = std::stoul(rangeValue.substr(0, dashPos));
                    if (dashPos < rangeValue.length() - 1) {
                        endPos = std::stoul(rangeValue.substr(dashPos + 1));
                    }
                }
            }
            
            size_t fileSize = std::filesystem::file_size(filePath);
            

            if (endPos == 0 || endPos >= fileSize) {
                endPos = fileSize - 1;
            }
            
            if (startPos >= fileSize || startPos > endPos) {
                res.status(416).send("Range Not Satisfiable"); 
            }
            
            size_t contentLength = endPos - startPos + 1;
            
            res.status(206);
            res.header("Content-Range", "bytes " + std::to_string(startPos) + "-" + 
                      std::to_string(endPos) + "/" + std::to_string(fileSize));
            res.header("Content-Length", std::to_string(contentLength));
            res.header("Accept-Ranges", "bytes");
            res.header("Content-Type", "application/octet-stream");
            
            std::ifstream file(filePath, std::ios::binary);
            file.seekg(startPos);
            
            std::string content;
            content.resize(contentLength);
            file.read(&content[0], contentLength);
            
            res.send(content);
        });
        
        app.get("/custom-stream", [](const boson::Request& req, boson::Response& res) {
            res.header("Content-Type", "text/plain");
            res.stream(true);
            
            for (int i = 1; i <= 10; i++) {
                std::string chunk = "Chunk " + std::to_string(i) + " of data\n";
                res.write(chunk);
                
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            
            res.end();
        });
        
        unsigned int port = 3000;
        std::string host = "127.0.0.1";
        app.configure(port, host);
        
        std::cout << "File Response Example server running at http://" << host << ":" << port << std::endl;
        
        return app.listen();
    }
    catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
}
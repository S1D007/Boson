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
        app.use(boson::StaticFiles::create(publicPath.string()));
        std::cout << "Serving static files from: " << publicPath << std::endl;
        
        std::filesystem::path uploadsDir = publicPath / "uploads";
        if (!std::filesystem::exists(uploadsDir)) {
            std::filesystem::create_directories(uploadsDir);
            std::cout << "Created uploads directory: " << uploadsDir << std::endl;
        }
        
        app.get("/", [](const boson::Request& req, boson::Response& res) {
            res.status(302).header("Location", "/index.html").send("");
        });
        
        app.post("/upload", [&uploadsDir](const boson::Request& req, boson::Response& res) {
            auto files = req.files();
            
            if (files.empty()) {
                res.status(400).jsonObject({
                    {"error", "No files uploaded"}
                });
                return;
            }
            
            std::vector<std::string> uploadedFiles;
            for (const auto& file : files) {
                std::string safeFileName = std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) 
                                         + "_" + file.fileName;
                std::filesystem::path savePath = uploadsDir / safeFileName;
                
                if (file.saveTo(savePath.string())) {
                    uploadedFiles.push_back(safeFileName);
                }
            }
            
            res.status(200).jsonObject({
                {"message", "Files uploaded successfully"},
                {"files", uploadedFiles},
                {"count", uploadedFiles.size()}
            });
        });
        
        app.get("/files", [&uploadsDir](const boson::Request& req, boson::Response& res) {
            std::vector<nlohmann::json> fileList;
            
            try {
                for (const auto& entry : std::filesystem::directory_iterator(uploadsDir)) {
                    if (std::filesystem::is_regular_file(entry)) {
                        std::string fileName = entry.path().filename().string();
                        std::string fileUrl = "/uploads/" + fileName;
                        
                        std::string extension = entry.path().extension().string();
                        std::transform(extension.begin(), extension.end(), extension.begin(),
                                    [](unsigned char c){ return std::tolower(c); });
                        
                        bool isImage = (extension == ".jpg" || extension == ".jpeg" || 
                                       extension == ".png" || extension == ".gif" || 
                                       extension == ".bmp" || extension == ".webp");
                                       
                        nlohmann::json fileInfo = {
                            {"name", fileName},
                            {"url", fileUrl},
                            {"isImage", isImage},
                            {"size", std::filesystem::file_size(entry)}
                        };
                        
                        fileList.push_back(fileInfo);
                    }
                }
                
                res.status(200).jsonObject({
                    {"files", fileList},
                    {"count", fileList.size()}
                });
            } catch (const std::exception& e) {
                res.status(500).jsonObject({
                    {"error", "Failed to list files"},
                    {"message", e.what()}
                });
            }
        });
        
        app.get("/uploads/:filename", [&uploadsDir](const boson::Request& req, boson::Response& res) {
            std::string filename = req.param("filename");
            std::filesystem::path filePath = uploadsDir / filename;
            
            if (filename.find("..") != std::string::npos) {
                res.status(403).send("Access denied");
                return;
            }
            
            if (!std::filesystem::exists(filePath) || !std::filesystem::is_regular_file(filePath)) {
                res.status(404).send("File not found");
                return;
            }
            
            std::ifstream file(filePath, std::ios::binary);
            if (!file) {
                res.status(500).send("Failed to read file");
                return;
            }
            
            std::string extension = filePath.extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(),
                          [](unsigned char c){ return std::tolower(c); });
                          
            std::string contentType = "application/octet-stream";
            
            if (extension == ".jpg" || extension == ".jpeg") contentType = "image/jpeg";
            else if (extension == ".png") contentType = "image/png";
            else if (extension == ".gif") contentType = "image/gif";
            else if (extension == ".bmp") contentType = "image/bmp";
            else if (extension == ".webp") contentType = "image/webp";
            else if (extension == ".pdf") contentType = "application/pdf";
            else if (extension == ".txt") contentType = "text/plain";
            else if (extension == ".html" || extension == ".htm") contentType = "text/html";
            else if (extension == ".css") contentType = "text/css";
            else if (extension == ".js") contentType = "application/javascript";
            else if (extension == ".json") contentType = "application/json";
            
            std::streamsize fileSize = std::filesystem::file_size(filePath);
            
            std::string content;
            content.resize(fileSize);
            file.read(&content[0], fileSize);
            
            res.header("Content-Type", contentType);
            res.header("Content-Disposition", "inline; filename=\"" + filename + "\"");
            res.send(content);
        });
        
        unsigned int port = 3000;
        std::string host = "127.0.0.1";
        app.configure(port, host);
        
        std::cout << "File Upload Example server running at http://" << host << ":" << port << std::endl;
        
        return app.listen();
    }
    catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
}
#include <boson/boson.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <string>

#include "controllers/hello_controller.hpp"

int main(int argc, char* argv[])
{
    try
    {

        std::srand(static_cast<unsigned int>(std::time(nullptr)));

        boson::initialize();

        boson::Server app;

        int port = 3000;
        std::string host = "127.0.0.1";

        for (int i = 1; i < argc; i++)
        {
            std::string arg = argv[i];
            if (arg.find("--port=") == 0)
            {
                port = std::stoi(arg.substr(7));
            }
            else if (arg.find("--host=") == 0)
            {
                host = arg.substr(7);
            }
        }

        app.configure(port, host);

        app.use(
            [](const boson::Request& req, boson::Response& res, boson::NextFunction& next)
            {
                std::cout << "[" << req.method() << "] " << req.path() << std::endl;
                next();
            });

        auto helloController = std::make_shared<HelloController>();
        app.registerController(helloController);

        std::cout << "Server starting at http://" << host << ":" << port << std::endl;
        return app.listen();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
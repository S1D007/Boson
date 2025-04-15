#include <boson/boson.hpp>
int main()
{
    boson::initialize();
    boson::Server app;
    app.get("/", [](const boson::Request& req, boson::Response& res) {
        res.headers({{"Content-Type", "text/html"}});
        res.send("<h1>Hello World</h1>");
    });

    app.get("/hello", [](const boson::Request& req, boson::Response& res) { 
        res.send("hello");
    });

    app.get("/json", [](const boson::Request& req, boson::Response& res) {
         res.jsonObject({{"msg", "json"}});
    });

    app.configure(3000, "127.0.0.1");

    return app.listen();
}

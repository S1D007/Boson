#include "boson/boson.hpp"
#include <iostream>
#include <string>

int main()
{
    boson::initialize();
    boson::Server server;
    server.configure(8080, "127.0.0.1");
    server.get("/api/user",
               [](const boson::Request& req, boson::Response& res)
               {
                   res.jsonObject({{"id", 1},
                                   {"name", "John Doe"},
                                   {"email", "john@example.com"},
                                   {"active", true},
                                   {"age", 30},
                                   {"roles", {"admin", "user", "moderator"}}});
               });
    server.get("/api/users",
               [](const boson::Request& req, boson::Response& res)
               {
                   res.jsonObject({{"users",
                                    {{{"id", 1}, {"name", "John"}, {"email", "john@example.com"}},
                                     {{"id", 2}, {"name", "Jane"}, {"email", "jane@example.com"}},
                                     {{"id", 3}, {"name", "Bob"}, {"email", "bob@example.com"}}}}});
               });
    server.get(
        "/api/complex",
        [](const boson::Request& req, boson::Response& res)
        {
            res.jsonObject(
                {{"user",
                  {{"id", 1},
                   {"name", "John Doe"},
                   {"contact", {{"email", "john@example.com"}, {"phone", "555-1234"}}}}},
                 {"metadata", {{"timestamp", "2025-04-15T12:00:00Z"}, {"version", "1.0"}}},
                 {"stats", {{"views", 1024}, {"likes", 42}, {"tags", {"api", "json", "c++"}}}}});
        });

    std::cout << "JSON Example Server running on http://localhost:8080" << std::endl;
    std::cout << "Available endpoints:" << std::endl;
    std::cout << "  - GET /api/user    - Single user example" << std::endl;
    std::cout << "  - GET /api/users   - Multiple users example" << std::endl;
    std::cout << "  - GET /api/complex - Complex nested JSON example" << std::endl;

    return server.listen();
}

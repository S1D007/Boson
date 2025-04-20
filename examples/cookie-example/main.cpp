#include "boson/boson.hpp"
#include <iostream>
#include <chrono>

class CookieController : public boson::Controller {
public:
    CookieController() = default;
    virtual ~CookieController() = default;
    
    std::string basePath() const override {
        return "/cookies";
    }
    
    void setCookie(const boson::Request& req, boson::Response& res) {
        res.cookie("simple_cookie", "hello world")
           .jsonObject({{"message", "Simple cookie set"}});
    }
    
    void setComplexCookie(const boson::Request& req, boson::Response& res) {
        auto expires = std::chrono::system_clock::now() + std::chrono::hours(24);
        
        res.cookie("complex_cookie", "secure_value", {
            {"path", "/"},
            {"domain", req.hostname()},
            {"maxAge", 86400},
            {"httpOnly", true},
            {"secure", req.secure()},
            {"sameSite", "Strict"},
            {"expires", expires}
        });
        
        res.jsonObject({{"message", "Complex cookie set with multiple options"}});
    }
    
    void readCookies(const boson::Request& req, boson::Response& res) {
        std::string simpleCookie = req.cookie("simple_cookie");
        std::string complexCookie = req.cookie("complex_cookie");
        
        auto allCookies = req.cookies();
        
        res.jsonObject({
            {"simple_cookie", simpleCookie},
            {"complex_cookie", complexCookie},
            {"all_cookies", allCookies},
            {"has_simple_cookie", !simpleCookie.empty()},
            {"cookie_count", allCookies.size()}
        });
    }
    
    void clearCookie(const boson::Request& req, boson::Response& res) {
        res.clearCookie("simple_cookie")
           .clearCookie("complex_cookie", {{"path", "/"}, {"domain", req.hostname()}})
           .jsonObject({{"message", "Cookies cleared"}});
    }
    
    void showUserPreference(const boson::Request& req, boson::Response& res) {
        std::string theme = req.cookie("theme");
        if (theme.empty()) {
            theme = "light";
        }
        
        res.jsonObject({
            {"theme", theme},
            {"message", "Current theme preference: " + theme}
        });
    }
    
    void setUserPreference(const boson::Request& req, boson::Response& res) {
        auto json = req.json();
        std::string theme = "light";
        
        if (json.contains("theme")) {
            theme = json["theme"];
        }
        
        res.cookie("theme", theme, {
            {"maxAge", 30 * 24 * 60 * 60},
            {"path", "/"}
        });
        
        res.jsonObject({
            {"theme", theme},
            {"message", "Theme preference saved"}
        });
    }
    
    void useProtocolAndUrl(const boson::Request& req, boson::Response& res) {
        res.jsonObject({
            {"protocol", req.protocol()},
            {"isSecure", req.secure()},
            {"hostname", req.hostname()},
            {"originalUrl", req.originalUrl()},
            {"method", req.method()}
        });
    }
};

int main() {
    boson::initialize();
    boson::Server app;
    
    app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
        std::cout << req.method() << " " << req.originalUrl() << " from " << req.hostname() 
                  << " via " << req.protocol() << std::endl;
        
        auto cookies = req.cookies();
        if (!cookies.empty()) {
            std::cout << "Cookies received: " << cookies.size() << std::endl;
        }
        
        next();
    });
    
    // Create a controller instance and router
    auto cookieController = std::make_shared<CookieController>();
    auto cookieRouter = boson::createRouter(cookieController);
    
    // Register routes on the router
    cookieRouter.get("/set", &CookieController::setCookie);
    cookieRouter.get("/set-complex", &CookieController::setComplexCookie);
    cookieRouter.get("/read", &CookieController::readCookies);
    cookieRouter.get("/clear", &CookieController::clearCookie);
    cookieRouter.get("/preferences", &CookieController::showUserPreference);
    cookieRouter.post("/preferences", &CookieController::setUserPreference);
    cookieRouter.get("/request-info", &CookieController::useProtocolAndUrl);
    
    // Mount the router to the server
    cookieRouter.mountOn(&app);
    
    // Root path redirects to the cookie examples
    app.get("/", [](const boson::Request& req, boson::Response& res) {
        res.redirect("/cookies/read");
    });
    
    app.configure(3000, "127.0.0.1");
    std::cout << "Cookie example server running on http://localhost:3000" << std::endl;
    
    // Start the server
    return app.listen();
}
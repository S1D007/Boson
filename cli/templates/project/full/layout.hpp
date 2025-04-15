#pragma once

#include <string>
#include <sstream>

/**
 * @class LayoutView
 * @brief Base class for views with common layout
 */
class LayoutView {
public:
    /**
     * @brief Render the layout with content
     * @param title The page title
     * @param content The page content
     * @return The rendered HTML
     */
    static std::string render(const std::string& title, const std::string& content) {
        std::stringstream ss;
        
        ss << "<!DOCTYPE html>\n"
           << "<html lang=\"en\">\n"
           << "<head>\n"
           << "    <meta charset=\"UTF-8\">\n"
           << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
           << "    <title>" << title << " - Boson App</title>\n"
           << "    <link rel=\"stylesheet\" href=\"/css/styles.css\">\n"
           << "</head>\n"
           << "<body>\n"
           << "    <header>\n"
           << "        <nav>\n"
           << "            <ul>\n"
           << "                <li><a href=\"/\">Home</a></li>\n"
           << "                <li><a href=\"/about\">About</a></li>\n"
           << "                <li><a href=\"/api\">API</a></li>\n"
           << "            </ul>\n"
           << "        </nav>\n"
           << "    </header>\n"
           << "    <main>\n"
           << content << "\n"
           << "    </main>\n"
           << "    <footer>\n"
           << "        <p>&copy; " << "2025" << " - Built with Boson Framework</p>\n"
           << "    </footer>\n"
           << "    <script src=\"/js/app.js\"></script>\n"
           << "</body>\n"
           << "</html>";
        
        return ss.str();
    }
};
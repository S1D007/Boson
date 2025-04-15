#include "boson/database/database.hpp"

#include <string>
#include <memory>
#include <functional>
#include <map>
#include <mutex>

namespace boson {
namespace db {


static std::map<std::string, std::function<std::shared_ptr<Database>()>> databaseRegistry;
static std::mutex registryMutex;

void DatabaseFactory::registerImplementation(const std::string& name, std::function<std::shared_ptr<Database>()> creator) {
    std::lock_guard<std::mutex> lock(registryMutex);
    databaseRegistry[name] = creator;
}

std::shared_ptr<Database> DatabaseFactory::getImplementation(const std::string& name) {
    std::lock_guard<std::mutex> lock(registryMutex);
    auto it = databaseRegistry.find(name);
    if (it != databaseRegistry.end()) {
        return it->second();
    }
    return nullptr;
}

} 
} 
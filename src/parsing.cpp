#include "parsing.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>

LocationConfig::LocationConfig()
    : autoindex(false), redirect_code(0), upload_enabled(false) {
      allowed_methods.push_back("GET");
}

ServerConfig::ServerConfig()
    : client_max_body_size(1048576) {}// Default 1MB

Config::Config() {}

Config::Config(const Config& other) : servers(other.servers), configFile(other.configFile) {}

Config& Config::operator=(const Config& other) {
    if (this != &other) {
        servers = other.servers;
        configFile = other.configFile;
    }
    return *this;
}

Config::Config(const std::string& config_file) {
    loadConfig(config_file);
}

Config::~Config() {}

void Config::loadConfig(const std::string& config_file) {
  configFile = config_file;
  servers.clear();
  parseFile(config_file);
  validateConfig();
}

const std::vector<ServerConfig>& Config::getServers() const {
    return servers;
}


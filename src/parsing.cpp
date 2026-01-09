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

std::vector<std::string> Config::tokenize(const std::string& content) {
  std::vector<std::string> tokens;
  std::string token;
  bool in_quotes = false;

  for (size_t i = 0; i < content.length(); ++i) {
    char c = content[i];
    if (c == '#' && !in_quotes) { // ignore comments
      while (i < content.length() && content[i] != '\n')
        ++i;
      continue;
    }
    if (c == '"') { // quotes
      in_quotes = !in_quotes;
      continue;
    }
    if (!in_quotes && (isspace(c) || c == '{' || c == '}' || c == ';')) { // delimiters
      if (!token.empty()) { // end of a token
        tokens.push_back(token);
        token.clear();
      }
      if (c == '{' || c == '}' || c == ';') { // single char tokens
        tokens.push_back(std::string(1, c));
      }
    } else {
      token += c;
    }
  }
  if (!token.empty()) { // last token
    tokens.push_back(token);
  }
  return tokens;
}
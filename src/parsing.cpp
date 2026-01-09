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

void Config::parseFile(const std::string& config_file) {
  std::ifstream file(config_file);
  if (!file.is_open())
    throw ConfigException("Could not open config file: " + config_file);

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string content = buffer.str();
  file.close();

  std::vector<std::string> tokens = tokenize(content);
  size_t index = 0;
  while (index < tokens.size()) {
    if (tokens[index] == "server") {
      parseServerBlock(tokens, index);
    } else {
      throw ConfigException("Unexpected token: " + tokens[index]);
    }
  }
}

std::string Config::getNextToken(std::vector<std::string>& tokens, size_t& pos){
  if (pos >= tokens.size())
    throw ConfigException("Unexpected end of configuration");
  return tokens[pos++];
}

void Config::expectToken(std::vector<std::string>& tokens, size_t& index, const std::string& expected) {
  std::string token = getNextToken(tokens, index);
  if (token != expected)
    throw ConfigException("Expected token: '" + expected + "', got: '" + token + "'");
}

size_t Config::parseSize(const std::string& str) {
  size_t value;
  char unit = 0;
  std::stringstream ss(str);

  ss >> value;
  if (!ss.eof())
    ss >> unit;

  switch (unit) {
    case 'K' : case 'k' : return value * 1024;
    case 'M' : case 'm' : return value * 1024 * 1024;
    case 'G' : case 'g' : return value * 1024 * 1024 * 1024;
    default : return value;
  }
}
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
    if (c == '#' && !in_quotes) {
      while (i < content.length() && content[i] != '\n')
        ++i;
      continue;
    }
    if (c == '"') {
      in_quotes = !in_quotes;
      continue;
    }
    if (!in_quotes && (isspace(c) || c == '{' || c == '}' || c == ';')) {
      if (!token.empty()) {
        tokens.push_back(token);
        token.clear();
      }
      if (c == '{' || c == '}' || c == ';') {
        tokens.push_back(std::string(1, c));
      }
    } else {
      token += c;
    }
  }
  if (!token.empty()) {
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

void Config::parseServerBlock(std::vector<std::string>& tokens, size_t& index) {
  ServerConfig server;

  ++index;
  expectToken(tokens, index, "{");

  while (index < tokens.size() && tokens[index] != "}") {
    std::string directive = getNextToken(tokens, index);

    if (directive == "listen") {
      std::string port = getNextToken(tokens, index);
      server.listen_ports.push_back(port);
      expectToken(tokens, index, ";");
    } else if (directive == "server_name") {
      server.server_name = getNextToken(tokens, index);
      expectToken(tokens, index, ";");
    } else if (directive == "error_page") {
      std::vector<int> codes;
      while (index < tokens.size() && tokens[index] != ";") {
        std::string token = tokens[index];
        if (token[0] >= '0' && token[0] <= '9') {
          codes.push_back(std::atoi(token.c_str()));
          ++index;
        } else {
          break;
        }
      }
      std::string page = getNextToken(tokens, index);
      for (size_t i = 0; i < codes.size(); ++i) {
        server.error_pages[std::to_string(codes[i])] = page;
      }
      expectToken(tokens, index, ";");
    } else if (directive == "client_max_body_size") {
      std::string size_str = getNextToken(tokens, index);
      server.client_max_body_size = parseSize(size_str);
      expectToken(tokens, index, ";");
    } else if (directive == "location") {
      parseLocationBlock(tokens, index, server);
    } else {
      throw ConfigException("Unknown directive in server block: " + directive);
    }
  }

  expectToken(tokens, index, "}");
  servers.push_back(server);
}

void Config::parseLocationBlock(std::vector<std::string>& lines, size_t& index, ServerConfig& server) {
  LocationConfig location;

  std::string path = getNextToken(lines, index);
  location.path = path;
  expectToken(lines, index, "{");

  while (index < lines.size() && lines[index] != "}") {
    std::string directive = getNextToken(lines, index);

    if (directive == "root") {
      location.root = getNextToken(lines, index);
      expectToken(lines, index, ";");
    } else if (directive == "index") {
      while (index < lines.size() && lines[index] != ";") {
        location.index.push_back(getNextToken(lines, index));
      }
      expectToken(lines, index, ";");
    } else if (directive == "allowed_methods") {
      while (index < lines.size() && lines[index] != ";") {
        location.allowed_methods.push_back(getNextToken(lines, index));
      }
      expectToken(lines, index, ";");
    } else if (directive == "autoindex") {
      std::string value = getNextToken(lines, index);
      location.autoindex = (value == "on");
      expectToken(lines, index, ";");
    } else if (directive == "redirect") {
      location.redirect_code = std::atoi(getNextToken(lines, index).c_str());
      location.redirect_url = getNextToken(lines, index);
      expectToken(lines, index, ";");
    } else if (directive == "upload_enable") {
      std::string value = getNextToken(lines, index);
      location.upload_enabled = (value == "on");
      expectToken(lines, index, ";");
    } else if (directive == "upload_store") {
      location.upload_store = getNextToken(lines, index);
      expectToken(lines, index, ";");
    } else if (directive == "cgi_pass") {
      std::string extension = getNextToken(lines, index);
      std::string path = getNextToken(lines, index);
      location.cgi_pass[extension] = path;
      expectToken(lines, index, ";");
    } else {
      throw ConfigException("Unknown directive in location block: " + directive);
    }
  }
  expectToken(lines, index, "}");
  server.locations.push_back(location);
}

void Config::validateConfig() const {
  if (servers.empty())
    throw ConfigException("No server blocks defined in configuration");

  for (size_t i = 0; i < servers.size(); ++i) {
    const ServerConfig& server = servers[i];
    if (server.listen_ports.empty())
      throw ConfigException("Server block missing 'listen' directive");

    for (size_t j = 0; j < server.locations.size(); ++j) {
      const LocationConfig& location = server.locations[j];
      if (location.path.empty())
        throw ConfigException("Location block missing path");
      if (location.root.empty())
        throw ConfigException("Location block missing 'root' directive for path: " + location.path);
    }
  }
}
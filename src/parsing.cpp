#include "parsing.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <set>

static int parseInt(const std::string& str, const std::string& context) {
  if (str.empty())
    throw Config::ConfigException("Empty value for " + context);

  char* end;
  errno = 0;
  long value = std::strtol(str.c_str(), &end, 10);

  if (errno == ERANGE || value > INT_MAX || value < INT_MIN)
    throw Config::ConfigException("Value out of range for " + context + ": " + str);
  if (*end != '\0')
    throw Config::ConfigException("Invalid integer for " + context + ": " + str);

  return static_cast<int>(value);
}

static void validatePort(const std::string& port_str) {
  int port = parseInt(port_str, "port");
  if (port < 1 || port > 65535)
    throw Config::ConfigException("Port number out of range (1-65535): " + port_str);
}

static bool isValidRedirectCode(int code) {
  return (code >= 300 && code <= 399) || code == 200 || code == 204;
}

static void validateHttpCode(int code, const std::string& context) {
  if (code < 100 || code > 599)
    throw Config::ConfigException("Invalid HTTP status code for " + context + ": " + std::to_string(code));
}

LocationConfig::LocationConfig()
    : autoindex(false), redirect_code(0), upload_enabled(false) {
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
  if (in_quotes)
    throw ConfigException("Unclosed quote in configuration file");
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
  if (str.empty())
    throw ConfigException("Empty value for size");

  char* end;
  errno = 0;
  unsigned long value = std::strtoul(str.c_str(), &end, 10);

  if (errno == ERANGE)
    throw ConfigException("Size value out of range: " + str);
  if (end == str.c_str())
    throw ConfigException("Invalid size value: " + str);

  char unit = *end;
  if (unit != '\0' && *(end + 1) != '\0')
    throw ConfigException("Invalid size unit: " + str);

  switch (unit) {
    case 'K' : case 'k' : return value * 1024;
    case 'M' : case 'm' : return value * 1024 * 1024;
    case 'G' : case 'g' : return value * 1024 * 1024 * 1024;
    case '\0' : return value;
    default :
      throw ConfigException("Unknown size unit '" + std::string(1, unit) + "' in: " + str);
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
      validatePort(port);
      server.listen_ports.push_back(port);
      expectToken(tokens, index, ";");
    } else if (directive == "server_name") {
      std::string name = getNextToken(tokens, index);
      if (name.empty() || name == ";")
        throw ConfigException("Empty value for server_name");
      server.server_name = name;
      expectToken(tokens, index, ";");
    } else if (directive == "error_page") {
      std::vector<int> codes;
      while (index < tokens.size() && tokens[index] != ";") {
        std::string token = tokens[index];
        if (!token.empty() && token[0] >= '0' && token[0] <= '9') {
          int code = parseInt(token, "error_page code");
          validateHttpCode(code, "error_page");
          codes.push_back(code);
          ++index;
        } else {
          break;
        }
      }
      if (codes.empty())
        throw ConfigException("error_page directive requires at least one status code");
      std::string page = getNextToken(tokens, index);
      if (page.empty() || page == ";")
        throw ConfigException("error_page directive requires a file path");
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
      std::string root = getNextToken(lines, index);
      if (root.empty() || root == ";")
        throw ConfigException("Empty value for root directive");
      location.root = root;
      expectToken(lines, index, ";");
    } else if (directive == "index") {
      while (index < lines.size() && lines[index] != ";") {
        std::string idx = getNextToken(lines, index);
        if (!idx.empty() && idx != ";")
          location.index.push_back(idx);
      }
      expectToken(lines, index, ";");
    } else if (directive == "allowed_methods") {
      location.allowed_methods.clear();
      while (index < lines.size() && lines[index] != ";") {
        std::string method = getNextToken(lines, index);
        if (!method.empty() && method != ";") {
          if (method != "GET" && method != "POST" && method != "DELETE" &&
              method != "PUT" && method != "HEAD" && method != "OPTIONS" &&
              method != "PATCH")
            throw ConfigException("Unknown HTTP method: " + method);
          location.allowed_methods.push_back(method);
        }
      }
      if (location.allowed_methods.empty())
        throw ConfigException("allowed_methods directive requires at least one method");
      expectToken(lines, index, ";");
    } else if (directive == "autoindex") {
      std::string value = getNextToken(lines, index);
      if (value != "on" && value != "off")
        throw ConfigException("autoindex must be 'on' or 'off', got: " + value);
      location.autoindex = (value == "on");
      expectToken(lines, index, ";");
    } else if (directive == "redirect") {
      int code = parseInt(getNextToken(lines, index), "redirect code");
      if (!isValidRedirectCode(code))
        throw ConfigException("Invalid redirect code: " + std::to_string(code));
      location.redirect_code = code;
      std::string url = getNextToken(lines, index);
      if (url.empty() || url == ";")
        throw ConfigException("redirect directive requires a URL");
      location.redirect_url = url;
      expectToken(lines, index, ";");
    } else if (directive == "return") {
      int code = parseInt(getNextToken(lines, index), "return code");
      if (!isValidRedirectCode(code))
        throw ConfigException("Invalid return code: " + std::to_string(code));
      location.redirect_code = code;
      std::string url = getNextToken(lines, index);
      if (url.empty() || url == ";")
        throw ConfigException("return directive requires a URL");
      location.redirect_url = url;
      expectToken(lines, index, ";");
    } else if (directive == "upload_enable") {
      std::string value = getNextToken(lines, index);
      if (value != "on" && value != "off")
        throw ConfigException("upload_enable must be 'on' or 'off', got: " + value);
      location.upload_enabled = (value == "on");
      expectToken(lines, index, ";");
    } else if (directive == "upload_store") {
      std::string store = getNextToken(lines, index);
      if (store.empty() || store == ";")
        throw ConfigException("Empty value for upload_store directive");
      location.upload_store = store;
      expectToken(lines, index, ";");
    } else if (directive == "cgi_pass") {
      std::string extension = getNextToken(lines, index);
      std::string cgi_path = getNextToken(lines, index);
      if (extension.empty() || cgi_path.empty())
        throw ConfigException("cgi_pass requires extension and path");
      location.cgi_pass[extension] = cgi_path;
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

  std::set<std::string> all_ports;

  for (size_t i = 0; i < servers.size(); ++i) {
    const ServerConfig& server = servers[i];
    if (server.listen_ports.empty())
      throw ConfigException("Server block missing 'listen' directive");

    // Check for duplicate ports within this server
    std::set<std::string> server_ports;
    for (size_t p = 0; p < server.listen_ports.size(); ++p) {
      const std::string& port = server.listen_ports[p];
      if (server_ports.count(port))
        throw ConfigException("Duplicate port in server block: " + port);
      server_ports.insert(port);

      // Check for duplicate ports across all servers
      if (all_ports.count(port))
        throw ConfigException("Port " + port + " is already used by another server block");
      all_ports.insert(port);
    }

    for (size_t j = 0; j < server.locations.size(); ++j) {
      const LocationConfig& location = server.locations[j];
      if (location.path.empty())
        throw ConfigException("Location block missing path");

      // Only require root for locations that are not pure redirects
      bool is_redirect = (location.redirect_code != 0);
      if (location.root.empty() && !is_redirect)
        throw ConfigException("Location block missing 'root' directive for path: " + location.path);

      // Validate that locations with upload_enabled have upload_store
      if (location.upload_enabled && location.upload_store.empty())
        throw ConfigException("Location with upload_enable requires upload_store for path: " + location.path);
    }
  }
}
#pragma once

#include <string>
#include <vector>
#include <map>

struct LocationConfig {
  std::string path;
  std::string root;
  std::vector<std::string> index;
  std::vector<std::string> allowed_methods;
  bool autoindex;
  std::string redirect_url;
  int redirect_code;
  bool upload_enabled;
  std::string upload_store;
  std::map<std::string, std::string> cgi_pass;

  LocationConfig();
};

struct ServerConfig {
  std::vector<std::string> listen_ports;
  std::vector<LocationConfig> locations;
  std::map<std::string, std::string> error_pages;
  std::string server_name;
  size_t client_max_body_size;

  ServerConfig();
};

class Config {
  public:
    Config();
    Config(const Config& other);
    Config& operator=(const Config& other);
    Config(const std::string& config_file);
    ~Config();

    void loadConfig(const std::string& config_file);
    const std::vector<ServerConfig>& getServers() const;

    class ConfigException : public std::exception {
      public:
        ConfigException();
        ConfigException(const std::string& msg);
        ConfigException(const ConfigException& other);
        ConfigException& operator=(const ConfigException& other);
        virtual ~ConfigException();
        virtual const char* what();
      private:
        std::string message;
      };

  private:
    std::vector<ServerConfig> servers;
    std::string configFile;

    void parseFile(const std::string& config_file);
    void parseServerBlock(std::vector<std::string>& lines, size_t& index);
    void parseLocationBlock(std::vector<std::string>& lines, size_t& index, ServerConfig& server);
    std::vector<std::string> tokenize(const std::string& content);
    void validateConfig() const;

    std::string getNextToken(std::vector<std::string>& tokens, size_t& pos);
    void expectToken(std::vector<std::string>& tokens, size_t& index, const std::string& expected);
    size_t parseSize(const std::string& str);
};
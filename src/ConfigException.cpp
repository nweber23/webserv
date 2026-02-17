#include "Parsing.hpp"

Config::ConfigException::ConfigException() : message("Configuration Exception") {}

Config::ConfigException::ConfigException(const std::string& msg) : message("Configuration Exception: " + msg) {}

Config::ConfigException::ConfigException(const ConfigException& other) : message(other.message) {}

Config::ConfigException& Config::ConfigException::operator=(const ConfigException& other) {
    if (this != &other) {
        message = other.message;
    }
    return *this;
}

Config::ConfigException::~ConfigException() {}

const char* Config::ConfigException::what() const noexcept {
    return message.c_str();
}
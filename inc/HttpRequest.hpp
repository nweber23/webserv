#ifndef __HTTPREQUEST_H
#define __HTTPREQUEST_H

#include <map>
#include <string>
#include "parsing.hpp"

struct HttpRequest
{
    std::string method;              // "GET", "POST", etc.
    std::string path;                // "/index.html"
    std::string query;               // "a=1&b=2" (after ?)
    std::string version;             // "HTTP/1.1"
    std::map<std::string, std::string> headers;
    std::string body;

    // Temporary solution
    const LocationConfig* location = nullptr;
};

#endif
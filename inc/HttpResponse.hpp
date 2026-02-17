#ifndef __HTTPRESPONSE_HPP
#define __HTTPRESPONSE_HPP

#include <string>
#include <map>

struct HttpResponse
{
    int status{200};
    std::string statusText{"OK"};
    std::map<std::string, std::string> headers;
    std::string body;
};

#endif
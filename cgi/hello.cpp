#include <cstdlib>
#include <iostream>

int main()
{
    const char* method = std::getenv("REQUEST_METHOD");
    const char* query  = std::getenv("QUERY_STRING");
    const char* cookie = std::getenv("HTTP_COOKIE");

    std::cout << "Content-Type: text/html\r\n\r\n";
    std::cout << "<html><body>";
    std::cout << "<h1>CPP CGI OK</h1>";
    std::cout << "<pre>";
    std::cout << "REQUEST_METHOD=" << (method ? method : "") << "\n";
    std::cout << "QUERY_STRING="  << (query  ? query  : "") << "\n";
    std::cout << "HTTP_COOKIE="   << (cookie ? cookie : "") << "\n";
    std::cout << "</pre>";
    std::cout << "</body></html>";
    return 0;
}

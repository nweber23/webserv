#include "middleware/CookieMiddleware.hpp"
#include "CookieHandler.hpp"
#include <sstream>

CookieMiddleware::CookieMiddleware(std::shared_ptr<ErrorPageHandler> errorHandler)
  : AMiddleware(errorHandler)
{}

std::string CookieMiddleware::_extractFormValue(const std::string& body, const std::string& fieldName)
{
  std::string search = fieldName + "=";
  size_t pos = body.find(search);

  if (pos == std::string::npos)
    return "";

  size_t start = pos + search.length();
  size_t end = body.find("&", start);
  if (end == std::string::npos)
    end = body.length();

  std::string value = body.substr(start, end - start);
  std::string decoded;
  for (size_t i = 0; i < value.length(); ++i)
  {
    if (value[i] == '+')
      decoded += ' ';
    else if (value[i] == '%' && i + 2 < value.length())
    {
      std::string hex = value.substr(i + 1, 2);
      char c = static_cast<char>(std::strtol(hex.c_str(), NULL, 16));
      decoded += c;
      i += 2;
    }
    else
      decoded += value[i];
  }

  return decoded;
}

bool CookieMiddleware::handle(HttpRequest& request, HttpResponse& response)
{
  if (request.method != "POST" || request.path != "/cookies.html")
    return callNext(request, response);

  std::string action = _extractFormValue(request.body, "action");
  std::string color = _extractFormValue(request.body, "theme_color");

  CookieHandler cookieHandler;

  if (action == "save" && !color.empty())
  {
    std::string setCookie = cookieHandler.buildSetCookieHeader("theme_color", color, "/");
    response.headers["Set-Cookie"] = setCookie;
  }
  else if (action == "clear")
  {
    std::string clearCookie = cookieHandler.buildClearCookieHeader("theme_color", "/");
    response.headers["Set-Cookie"] = clearCookie;
  }

  return callNext(request, response);
}

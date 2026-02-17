#include "CookieHandler.hpp"
#include <sstream>
#include <algorithm>
#include <ctime>
#include <iomanip>

CookieHandler::CookieHandler() {}

CookieHandler::CookieHandler(const CookieHandler& other) {
  (void)other;
}

CookieHandler& CookieHandler::operator=(const CookieHandler& other) {
  if (this != &other)
    (void)other;
  return *this;
}

CookieHandler::~CookieHandler() {}

std::map<std::string, std::string> CookieHandler::parseCookies(const std::string& cookieHeader) {
  std::map<std::string, std::string> cookies;
  if (cookieHeader.empty())
    return cookies;
  std::istringstream stream(cookieHeader);
  std::string pair;
  while (std::getline(stream, pair, ';')) {
    pair = trimWhitespace(pair);
    size_t pos = pair.find('=');
    if (pos != std::string::npos) {
      std::string name = pair.substr(0, pos);
      std::string value = pair.substr(pos + 1);
      name = trimWhitespace(name);
      value = trimWhitespace(value);
      cookies[name] = value;
    }
  }
  return cookies;
}

std::string CookieHandler::getCookieValue(const std::string& cookieHeader, const std::string& name) {
  std::string search = name + "=";
  size_t pos = cookieHeader.find(search);

  if (pos != std::string::npos) {
    size_t start = pos + search.length();
    size_t end = cookieHeader.find(";", start);
    if (end == std::string::npos)
      end = cookieHeader.length();
    return cookieHeader.substr(start, end - start);
  }
  return "";
}

std::string CookieHandler::buildSetCookieHeader(
  const std::string& name,
  const std::string& value,
  const std::string& path,
  const std::string& expires,
  bool httpOnly,
  bool secure) {
  std::string setCookie = name + "=" + value;

  if (!path.empty())
    setCookie += "; Path=" + path;
  if (!expires.empty())
    setCookie += "; Expires=" + expires;
  if (httpOnly)
    setCookie += "; HttpOnly";
  if (secure)
    setCookie += "; Secure";
  return setCookie;
}

std::string CookieHandler::buildClearCookieHeader(const std::string& name, const std::string& path)
{
  // Set expiration to past date to clear cookie
  return buildSetCookieHeader(name, "", path, "Thu, 01 Jan 1970 00:00:00 GMT");
}

std::string CookieHandler::trimWhitespace(const std::string& str) {
  size_t first = str.find_first_not_of(" \t\n\r");
  if (first == std::string::npos)
    return "";
  size_t last = str.find_last_not_of(" \t\n\r");
  return str.substr(first, last - first + 1);
}

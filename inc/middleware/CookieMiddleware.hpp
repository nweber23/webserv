#pragma once

#include "AMiddleware.hpp"
#include <string>

class CookieMiddleware : public AMiddleware
{
private:
  std::string _extractFormValue(const std::string& body, const std::string& fieldName);

public:
  CookieMiddleware() = default;
  CookieMiddleware(const CookieMiddleware& other) = delete;
  CookieMiddleware& operator=(const CookieMiddleware& other) = delete;
  ~CookieMiddleware() override = default;

  CookieMiddleware(std::shared_ptr<ErrorPageHandler> errorHandler);

  bool handle(HttpRequest& request, HttpResponse& response) override;
};

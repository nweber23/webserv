#include "middleware/BodySizeValidationMiddleware.hpp"
#include <cstdlib>

#include <cctype>

BodySizeValidationMiddleware::BodySizeValidationMiddleware(const ServerConfig& config)
  : _config(config)
{}

static bool equalsIgnoreCase(const std::string& a, const std::string& b)
{
  if (a.size() != b.size())
    return false;
  for (std::size_t i = 0; i < a.size(); ++i) {
    if (std::tolower(static_cast<unsigned char>(a[i])) !=
        std::tolower(static_cast<unsigned char>(b[i]))) {
      return false;
    }
  }
  return true;
}

bool BodySizeValidationMiddleware::handle(HttpRequest& request, HttpResponse& response)
{
  // Perform a case-insensitive search for the Content-Length header
  auto it = request.headers.end();
  for (auto hit = request.headers.begin(); hit != request.headers.end(); ++hit) {
    if (equalsIgnoreCase(hit->first, "Content-Length")) {
      it = hit;
      break;
    }
  }
  std::string contentLengthStr = it->second;
  char* endptr;
  size_t contentLength = std::strtoul(contentLengthStr.c_str(), &endptr, 10);

  if (*endptr == '\0' && contentLength > _config.client_max_body_size) {
    response.status = 413;
    response.statusText = "Payload Too Large";
    response.body = "<h1>413 — Payload Too Large</h1>";
    response.headers["Content-Type"] = "text/html";
    response.headers["Content-Length"] = std::to_string(response.body.size());
    return true;
  }

  if (request.body.size() > _config.client_max_body_size) {
    response.status = 413;
    response.statusText = "Payload Too Large";
    response.body = "<h1>413 - Payload Too Large<h1>";
    response.headers["Content-Type"] = "text/html";
    response.headers["Content-Length"] = std::to_string(response.body.size());
    return true;
  }

  return callNext(request, response);
}

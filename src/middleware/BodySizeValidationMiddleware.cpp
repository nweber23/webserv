#include "middleware/BodySizeValidationMiddleware.hpp"
#include <cstdlib>
#include <cerrno>
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
  // Check actual body size first
  if (request.body.size() > _config.client_max_body_size) {
    response.status = 413;
    response.statusText = "Payload Too Large";
    response.body = "<h1>413 — Payload Too Large</h1>";
    response.headers["Content-Type"] = "text/html";
    response.headers["Content-Length"] = std::to_string(response.body.size());
    return true;
  }

  // Perform a case-insensitive search for the Content-Length header
  auto it = request.headers.end();
  for (auto hit = request.headers.begin(); hit != request.headers.end(); ++hit) {
    if (equalsIgnoreCase(hit->first, "Content-Length")) {
      it = hit;
      break;
    }
  }

  // If Content-Length header is present, validate it
  if (it != request.headers.end()) {
    const std::string& contentLengthStr = it->second;

    // Reject empty Content-Length
    if (contentLengthStr.empty()) {
      response.status = 400;
      response.statusText = "Bad Request";
      response.body = "<h1>400 — Bad Request</h1><p>Invalid Content-Length header</p>";
      response.headers["Content-Type"] = "text/html";
      response.headers["Content-Length"] = std::to_string(response.body.size());
      return true;
    }

    // Reject if starts with '-' (negative value)
    if (contentLengthStr[0] == '-') {
      response.status = 400;
      response.statusText = "Bad Request";
      response.body = "<h1>400 — Bad Request</h1><p>Invalid Content-Length header</p>";
      response.headers["Content-Type"] = "text/html";
      response.headers["Content-Length"] = std::to_string(response.body.size());
      return true;
    }

    char* endptr;
    errno = 0;  // Clear errno before strtoul
    unsigned long contentLength = std::strtoul(contentLengthStr.c_str(), &endptr, 10);

    // Check for conversion errors:
    // 1. No digits parsed (endptr == start)
    // 2. Trailing non-whitespace (endptr != end of string)
    // 3. Overflow (errno == ERANGE)
    if (endptr == contentLengthStr.c_str() || *endptr != '\0' || errno == ERANGE) {
      response.status = 400;
      response.statusText = "Bad Request";
      response.body = "<h1>400 — Bad Request</h1><p>Invalid Content-Length header</p>";
      response.headers["Content-Type"] = "text/html";
      response.headers["Content-Length"] = std::to_string(response.body.size());
      return true;
    }

    // Check if Content-Length exceeds limit
    if (contentLength > _config.client_max_body_size) {
      response.status = 413;
      response.statusText = "Payload Too Large";
      response.body = "<h1>413 — Payload Too Large</h1>";
      response.headers["Content-Type"] = "text/html";
      response.headers["Content-Length"] = std::to_string(response.body.size());
      return true;
    }
  }

  return callNext(request, response);
}

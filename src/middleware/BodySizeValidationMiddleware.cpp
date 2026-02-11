#include "middleware/BodySizeValidationMiddleware.hpp"
#include <sstream>
#include <cstdlib>

BodySizeValidationMiddleware::BodySizeValidationMiddleware(const ServerConfig& config)
  : _config(config)
{}

bool BodySizeValidationMiddleware::handle(HttpRequest& request, HttpResponse& response)
{
  auto it = request.headers.find("Content-Length");
  if (it != request.headers.end()) {
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
  }

  return callNext(request, response);
}

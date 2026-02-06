#include "middleware/LocationRouter.hpp"

LocationRouter::LocationRouter(const std::vector<LocationConfig>& locations)
	: _locations(locations) {}

// A location path matches a request path if:
//   - locationPath == "/"  (catch-all), or
//   - requestPath starts with locationPath AND the next char
//     is either '/' or end-of-string (boundary check so that
//     "/uploads" does NOT match "/uploadsomething").
bool LocationRouter::_matchesPrefix(const std::string& locationPath,
                                   const std::string& requestPath)
{
	if (locationPath == "/")
		return true;
	if (requestPath.compare(0, locationPath.size(), locationPath) != 0)
		return false;
	return requestPath.size() == locationPath.size()
	    || requestPath[locationPath.size()] == '/';
}

bool LocationRouter::handle(HttpRequest& request, HttpResponse& response)
{
	const LocationConfig* best = nullptr;
	size_t bestLen = 0;

	for (const auto& location : _locations)
	{
		if (_matchesPrefix(location.path, request.path))
		{
			best = &location;
			break;
		}
	}

	if (!best) {
		response.status = 404;
		response.statusText = "Not Found";
		response.body = "<h1>404 Not Found</h1>";
		response.headers["Content-Type"] = "text/html";
		return true;
	}

	request.location = best;
	return callNext(request, response);
}

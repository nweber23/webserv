#include "middleware/LocationRouter.hpp"

LocationRouter::LocationRouter(
	const std::vector<LocationConfig>& locations,
	std::shared_ptr<ErrorPageHandler> errorHandler)
	: AMiddleware(errorHandler), _locations(locations)
	{}

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
			// Choose the longest matching prefix (most specific)
			if (location.path.size() > bestLen)
			{
				best = &location;
				bestLen = location.path.size();
			}
		}
	}

	if (!best) {
		_errorHandler->buildErrorResponse(NotFound, response);
		return true;
	}

	request.location = best;
	return callNext(request, response);
}

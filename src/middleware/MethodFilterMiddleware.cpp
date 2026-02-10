#include "middleware/MethodFilterMiddleware.hpp"
#include "parsing.hpp"

bool MethodFilterMiddleware::handle(HttpRequest& request, HttpResponse& response)
{
	if (!request.location)
		return callNext(request, response);

	const auto& methods = request.location->allowed_methods;
	if (methods.empty())
		return callNext(request, response);

	for (const auto& m : methods)
	{
		if (m == request.method)
			return callNext(request, response);
	}

	// Method not allowed — build Allow header
	response.status = 405;
	response.statusText = "Method Not Allowed";
	std::string allow;
	for (size_t i = 0; i < methods.size(); ++i)
	{
		if (i > 0)
			allow += ", ";
		allow += methods[i];
	}
	response.headers["Allow"] = allow;
	response.headers["Content-Type"] = "text/html";
	response.body = "<h1>405 Method Not Allowed</h1>";
	return true;
}

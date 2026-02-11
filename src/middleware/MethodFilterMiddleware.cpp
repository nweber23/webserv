#include "middleware/MethodFilterMiddleware.hpp"
#include "parsing.hpp"

MethodFilterMiddleware::MethodFilterMiddleware(
	std::shared_ptr<ErrorPageHandler> errorHandler) : AMiddleware(errorHandler)
{}

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

	_errorHandler->buildErrorResponse(MethodNotAllowed, response);
	return true;
}

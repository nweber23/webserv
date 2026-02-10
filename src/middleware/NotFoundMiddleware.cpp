#include "middleware/NotFoundMiddleware.hpp"

NotFoundMiddleware::NotFoundMiddleware(
	std::shared_ptr<ErrorPageHandler> errorHandler) :
	AMiddleware(errorHandler)
{}

bool NotFoundMiddleware::handle(HttpRequest& request, HttpResponse& response)
{
	(void) request;

	_errorHandler->buildErrorResponse(NotFound, response);
	return true;
}

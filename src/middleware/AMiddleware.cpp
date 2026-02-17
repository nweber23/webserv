#include "AMiddleware.hpp"

bool AMiddleware::callNext(HttpRequest& request, HttpResponse& response) {
	if (_next)
		return _next->handle(request, response);
	return false;
}

void AMiddleware::setNext(std::unique_ptr<IMiddleware> next)
{
	_next = std::move(next);
}
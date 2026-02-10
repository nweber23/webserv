#include "middleware/NotFoundMiddleware.hpp"

bool NotFoundMiddleware::handle(HttpRequest& request, HttpResponse& response)
{
	(void) request;

	HttpResponse notFound;
	notFound.status = 404;
	notFound.statusText = "Not Found";
	notFound.headers["Content-Type"] = "text/html; charset=utf-8";
	notFound.body = "<html><body><h1>404 Not Found</h1></body></html>";

	response = notFound;
	return true;
}

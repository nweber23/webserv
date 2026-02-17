#include "middleware/RedirectMiddleware.hpp"
#include "Parsing.hpp"

bool RedirectMiddleware::handle(HttpRequest& request, HttpResponse& response)
{
	if (!request.location || request.location->redirect_code == 0)
		return callNext(request, response);

	response.status = request.location->redirect_code;
	response.headers["Location"] = request.location->redirect_url;

	switch (response.status)
	{
		case 300: response.statusText = "Multiple Choices";    break;
		case 301: response.statusText = "Moved Permanently";   break;
		case 302: response.statusText = "Found";               break;
		case 303: response.statusText = "See Other";           break;
		case 304: response.statusText = "Not Modified";        break;
		case 307: response.statusText = "Temporary Redirect";  break;
		case 308: response.statusText = "Permanent Redirect";  break;
		default:  response.statusText = "Redirect";            break;
	}
	response.body = "";
	return true;
}

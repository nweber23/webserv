#include "HttpApp.hpp"
#include <fstream>
#include <sstream>

HttpApp::HttpApp(const ServerConfig& config)
	: _config(config), _head(nullptr), _tail(nullptr)
{}

void HttpApp::use(std::unique_ptr<IMiddleware> middleware)
{
	if (!_head) {
		_tail = middleware.get();
		_head = std::move(middleware);
	} else {
		auto tail = middleware.get();
		_tail->setNext(std::move(middleware));
		_tail = tail;
	}
}

HttpResponse HttpApp::handle(HttpRequest& request)
{
	HttpResponse response;

	if (_head)
	{
		_head->handle(request, response);
	}
	else
	{
		response.status = 500;
		response.statusText = "Internal Server Error";
		response.body = "<h1>500 — No handlers configured</h1>";
	}

	applyErrorPages(response);
	return response;
}

void HttpApp::applyErrorPages(HttpResponse& response) const
{
	std::string code = std::to_string(response.status);
	auto it = _config.error_pages.find(code);
	if (it == _config.error_pages.end())
		return;

	std::ifstream file(it->second);
	if (!file.is_open())
		return;

	std::ostringstream ss;
	ss << file.rdbuf();
	response.body = ss.str();
	response.headers["Content-Type"] = "text/html";
	response.headers["Content-Length"] = std::to_string(response.body.size());
}

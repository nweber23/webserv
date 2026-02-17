#pragma once

#include "IHttpApp.hpp"
#include "Parsing.hpp"
#include <string>

class HttpApp : public IHttpApp
{
private:
	const ServerConfig& _config;
	std::unique_ptr<IMiddleware> _head;
	IMiddleware *_tail; // This variable just contain address. Is should be freed.

	void applyErrorPages(HttpResponse& response) const;

public:
	HttpApp() = delete;
	HttpApp(const HttpApp& other) = delete;
	const HttpApp& operator=(const HttpApp& other) = delete;

	HttpApp(const ServerConfig& config);
	~HttpApp() override = default;

	HttpResponse handle(HttpRequest& request) override;
	void use(std::unique_ptr<IMiddleware> middleware) override;
};

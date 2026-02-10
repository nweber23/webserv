#ifndef __IHTTPAPP_H
#define __IHTTPAPP_H

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "IMiddleware.hpp"
#include <memory>

class IHttpApp
{
private:
public:
	IHttpApp() = default;
	IHttpApp(const IHttpApp& other) = default;
	IHttpApp& operator=(const IHttpApp& other) = default;
	virtual ~IHttpApp() = default;

	virtual HttpResponse handle(HttpRequest& request) =0;
	virtual void use(std::unique_ptr<IMiddleware> middleware) =0;
};

#endif
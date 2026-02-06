#ifndef __IMIDDLEWARE_HPP
#define __IMIDDLEWARE_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <memory>


// Chain of Responsibility design pattern
class IMiddleware
{
private:
	IMiddleware(const IMiddleware& ather) = delete;
	IMiddleware& operator=(const IMiddleware& ather) = delete;

public:
	IMiddleware() = default;
	virtual ~IMiddleware() = default;

	IMiddleware(IMiddleware&& ather) = default;
    IMiddleware& operator=(IMiddleware&& ather) = default;

	virtual void setNext(std::unique_ptr<IMiddleware> next) =0;
	virtual bool handle(HttpRequest& request, HttpResponse& response) =0; 
};

#endif
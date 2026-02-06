#pragma once

#include "IMiddleware.hpp"

class AMiddleware : public IMiddleware
{
protected:
	std::unique_ptr<IMiddleware> _next;

	bool callNext(HttpRequest& request, HttpResponse& response);

public:
	void setNext(std::unique_ptr<IMiddleware> next) override;
};

#pragma once

#include "IMiddleware.hpp"


// TODO: Is this constructors okay?? May be not. Check in the next versions
class AMiddleware : public IMiddleware
{
protected:
	std::unique_ptr<IMiddleware> _next;

	bool callNext(HttpRequest& request, HttpResponse& response);

public:
	AMiddleware() = default;
	AMiddleware(const AMiddleware& other) = delete;
	AMiddleware& operator=(const AMiddleware& other) = delete;
	~AMiddleware() override = default;

	void setNext(std::unique_ptr<IMiddleware> next) override;
};

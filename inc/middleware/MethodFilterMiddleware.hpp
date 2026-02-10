#pragma once

#include "AMiddleware.hpp"

// Rejects requests whose HTTP method is not in the location's
// allowed_methods list.  Returns 405 with an Allow header.
class MethodFilterMiddleware : public AMiddleware
{
private:

public:
	MethodFilterMiddleware() = default;
	MethodFilterMiddleware(const MethodFilterMiddleware& other) = delete;
	MethodFilterMiddleware& operator=(const MethodFilterMiddleware& other) = delete;
	~MethodFilterMiddleware() override = default;

	MethodFilterMiddleware(std::shared_ptr<ErrorPageHandler> errorHandler);

	bool handle(HttpRequest& request, HttpResponse& response) override;
};

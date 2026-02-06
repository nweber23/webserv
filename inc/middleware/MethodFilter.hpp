#pragma once

#include "AMiddleware.hpp"

// Rejects requests whose HTTP method is not in the location's
// allowed_methods list.  Returns 405 with an Allow header.
class MethodFilter : public AMiddleware
{
private:

public:
	MethodFilter() = default;
	MethodFilter(const MethodFilter& other) = delete;
	MethodFilter& operator=(const MethodFilter& other) = delete;
	~MethodFilter() override = default;

	bool handle(HttpRequest& request, HttpResponse& response) override;
};

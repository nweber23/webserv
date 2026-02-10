#pragma once

#include "AMiddleware.hpp"

// If the matched location has a redirect (return 301 …), send
// the redirect response immediately without calling downstream.
class RedirectMiddleware : public AMiddleware
{
public:
	RedirectMiddleware() = default;
	RedirectMiddleware(const RedirectMiddleware& other) = delete;
	RedirectMiddleware& operator=(const RedirectMiddleware& other) = delete;
	~RedirectMiddleware() override = default;

	bool handle(HttpRequest& request, HttpResponse& response) override;
};

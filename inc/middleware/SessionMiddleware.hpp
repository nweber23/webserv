#pragma once

#include "AMiddleware.hpp"
#include "CookieHandler.hpp"

#include <string>
#include <memory>


class SessionMiddleware : public AMiddleware
{
private:
	std::string _makeSid();
	std::string _htmlEscape(const std::string& s);

public:
	SessionMiddleware() = default;
	SessionMiddleware(std::shared_ptr<ErrorPageHandler> errorHandler);
	SessionMiddleware(const SessionMiddleware& other) = delete;
	SessionMiddleware& operator=(const SessionMiddleware& other) = delete;
	~SessionMiddleware() override = default;

	bool handle(HttpRequest& request, HttpResponse& response) override;
};

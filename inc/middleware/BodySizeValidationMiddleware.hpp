#pragma once

#include "AMiddleware.hpp"
#include "Parsing.hpp"

// Validates that request body size does not exceed client_max_body_size
// Returns 413 Payload Too Large if exceeded
class BodySizeValidationMiddleware : public AMiddleware
{
private:
	const ServerConfig& _config;

public:
	BodySizeValidationMiddleware() = delete;
	BodySizeValidationMiddleware(const BodySizeValidationMiddleware& other) = delete;
	BodySizeValidationMiddleware& operator=(const BodySizeValidationMiddleware& other) = delete;
	~BodySizeValidationMiddleware() override = default;

	explicit BodySizeValidationMiddleware(const ServerConfig& config);
	bool handle(HttpRequest& request, HttpResponse& response) override;
};

#pragma once

#include "AMiddleware.hpp"
#include "parsing.hpp"
#include <vector>
#include <string>

class LocationRouter : public AMiddleware
{
private:
	const std::vector<LocationConfig>& _locations;

	static bool _matchesPrefix(const std::string& locationPath,
	                          const std::string& requestPath);

public:
	LocationRouter() = delete;
	LocationRouter(const LocationRouter& other) = delete;
	LocationRouter &operator=(const LocationRouter& other) = delete;
	~LocationRouter() override = default;

	explicit LocationRouter(const std::vector<LocationConfig>& locations);
	bool handle(HttpRequest& request, HttpResponse& response) override;
};

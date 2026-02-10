#pragma once

#include "AMiddleware.hpp"
#include <string>

// Serves static files from the location's root directory.
// Handles index files and autoindex directory listings.
class StaticFileMiddleware : public AMiddleware {
private:
	std::string resolveFilePath(const HttpRequest& request) const;
	bool serveFile(const std::string& filePath,
	               HttpResponse& response) const;
	bool generateDirectoryListing(const std::string& dirPath,
	                              const std::string& requestPath,
	                              HttpResponse& response) const;
	static std::string guessMimeType(const std::string& path);

public:
	StaticFileMiddleware() = default;
	StaticFileMiddleware(const StaticFileMiddleware& other) = delete;
	StaticFileMiddleware& operator=(const StaticFileMiddleware& other) = delete;
	~StaticFileMiddleware() override = default;

	bool handle(HttpRequest& request, HttpResponse& response) override;
};

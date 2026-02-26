#pragma once

#include "AMiddleware.hpp"
#include "ErrorPageHandler.hpp"

#include <string>
#include <vector>

class CgiHandler : public AMiddleware
{
public:
	CgiHandler() = delete;
	CgiHandler(std::shared_ptr<ErrorPageHandler> errorHandler);
	CgiHandler(const CgiHandler& other) = delete;
	CgiHandler& operator=(const CgiHandler& other) = delete;
	~CgiHandler() override = default;

	bool handle(HttpRequest& request, HttpResponse& response) override;

private:
	static std::string resolveScriptPath(const HttpRequest& request);
	static std::string getFileExtension(const std::string& path);
	static std::string getHeaderValue(const HttpRequest& request, const std::string& key);
	static std::string trim(const std::string& s);

	bool runCgi(const HttpRequest& request,
	           const std::string& interpreter,
	           const std::string& scriptPath,
	           std::string& cgiOutput);

	std::vector<std::string> buildEnv(const HttpRequest& request,
	                                 const std::string& scriptPath) const;

	bool parseCgiOutput(const std::string& raw, HttpResponse& response);
};

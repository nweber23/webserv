#pragma once

#include "AMiddleware.hpp"
#include <string>

// Handles file uploads (POST) and deletions (DELETE) for
// locations that have upload_enabled = true.
class UploadMiddleware : public AMiddleware
{
private:
	bool _handleUpload(const HttpRequest& request,
	                  HttpResponse& response);
	bool _handleDelete(const HttpRequest& request,
	                  HttpResponse& response);
	bool _isDirExist(const std::string& directory);

	std::string _extractUploadPath(const HttpRequest& request);
	bool _saveData(const std::string &filename, const HttpRequest& request);

	void _setErrorResponse(HttpResponse& response);
	void _setSuccessResponse(HttpResponse& response, std::string& fullPath);

public:
	UploadMiddleware() = default;
	UploadMiddleware(const UploadMiddleware& other) = delete;
	UploadMiddleware& operator=(const UploadMiddleware& other) = delete;
	~UploadMiddleware() override = default;

	bool handle(HttpRequest& request, HttpResponse& response) override;
};

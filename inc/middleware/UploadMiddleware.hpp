#pragma once

#include "AMiddleware.hpp"
#include <string>

// Handles file uploads (POST) and deletions (DELETE) for
// locations that have upload_enabled = true.
class UploadMiddleware : public AMiddleware {
private:
	bool _handleUpload(const HttpRequest& request,
	                  HttpResponse& response) const;
	bool _handleDelete(const HttpRequest& request,
	                  HttpResponse& response) const;
	bool _isDirExist(const std::string& direcotry) const;

public:
	UploadMiddleware() = default;
	UploadMiddleware(const UploadMiddleware& other) = delete;
	UploadMiddleware& operator=(const UploadMiddleware& other) = delete;
	~UploadMiddleware() override = default;

	bool handle(HttpRequest& request, HttpResponse& response) override;
};

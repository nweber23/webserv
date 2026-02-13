#include "middleware/StaticFileMiddleware.hpp"
#include "Parsing.hpp"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>

StaticFileMiddleware::StaticFileMiddleware(
	std::shared_ptr<ErrorPageHandler> errorHandler) : AMiddleware(errorHandler)
{}

// Maps the request URL to a filesystem path:
//   root + (requestPath minus locationPrefix)
std::string StaticFileMiddleware::resolveFilePath(const HttpRequest& request) const {
	const auto* loc = request.location;
	std::string root = loc->root;
	std::string relPath = request.path.substr(loc->path.size());
	if (relPath.empty() || relPath[0] != '/')
		relPath = "/" + relPath;
	return root + relPath;
}

std::string StaticFileMiddleware::guessMimeType(const std::string& path) {
	size_t dot = path.rfind('.');
	if (dot == std::string::npos) return "application/octet-stream";
	std::string ext = path.substr(dot);
	if (ext == ".html" || ext == ".htm") return "text/html";
	if (ext == ".css")  return "text/css";
	if (ext == ".js")   return "application/javascript";
	if (ext == ".json") return "application/json";
	if (ext == ".png")  return "image/png";
	if (ext == ".jpg"  || ext == ".jpeg") return "image/jpeg";
	if (ext == ".gif")  return "image/gif";
	if (ext == ".svg")  return "image/svg+xml";
	if (ext == ".txt")  return "text/plain";
	if (ext == ".xml")  return "application/xml";
	if (ext == ".pdf")  return "application/pdf";
	if (ext == ".ico")  return "image/x-icon";
	return "application/octet-stream";
}

bool StaticFileMiddleware::serveFile(const std::string& filePath,
                                  HttpResponse& response) const {
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open()) {
		_errorHandler->buildErrorResponse(NotFound, response);
		return true;
	}
	std::ostringstream ss;
	ss << file.rdbuf();
	response.status = 200;
	response.statusText = "OK";
	response.body = ss.str();
	response.headers["Content-Type"]   = guessMimeType(filePath);
	response.headers["Content-Length"] = std::to_string(response.body.size());
	return true;
}

bool StaticFileMiddleware::generateDirectoryListing(
		const std::string& dirPath,
		const std::string& requestPath,
		HttpResponse& response) const {
	DIR* dir = opendir(dirPath.c_str());
	if (!dir) {
		_errorHandler->buildErrorResponse(InternalServerError, response);
		return true;
	}

	std::string urlPath = requestPath;
	if (urlPath.empty() || urlPath.back() != '/') urlPath += '/';

	std::ostringstream html;
	html << "<html><head><title>Index of " << urlPath
	     << "</title></head><body>\n";
	html << "<h1>Index of " << urlPath << "</h1><hr><pre>\n";

	if (urlPath != "/")
		html << "<a href=\"../\">../</a>\n";

	struct dirent* entry;
	while ((entry = readdir(dir)) != nullptr) {
		std::string name = entry->d_name;
		if (name == "." || name == "..") continue;

		std::string fullPath = dirPath + "/" + name;
		struct stat st;
		if (stat(fullPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
			name += "/";

		html << "<a href=\"" << urlPath << name << "\">"
		     << name << "</a>\n";
	}
	closedir(dir);

	html << "</pre><hr></body></html>\n";
	response.status = 200;
	response.statusText = "OK";
	response.body = html.str();
	response.headers["Content-Type"]   = "text/html";
	response.headers["Content-Length"] = std::to_string(response.body.size());
	return true;
}

bool StaticFileMiddleware::handle(HttpRequest& request, HttpResponse& response) {
	if (!request.location)
		return callNext(request, response);

	// Skip — let CgiHandler deal with CGI locations
	if (!request.location->cgi_pass.empty())
		return callNext(request, response);

	// Skip — let UploadMiddleware deal with upload POST/DELETE
	if (request.location->upload_enabled
	    && (request.method == "POST" || request.method == "DELETE"))
		return callNext(request, response);

	std::string filePath = resolveFilePath(request);

	struct stat st;
	if (stat(filePath.c_str(), &st) != 0) {
		_errorHandler->buildErrorResponse(NotFound, response);
		return true;
	}

	if (S_ISDIR(st.st_mode)) {
		// Try each configured index file
		for (const auto& idx : request.location->index) {
			std::string indexPath = filePath;
			if (indexPath.back() != '/') indexPath += '/';
			indexPath += idx;
			struct stat ist;
			if (stat(indexPath.c_str(), &ist) == 0 && S_ISREG(ist.st_mode))
				return serveFile(indexPath, response);
		}
		// Fall back to autoindex if enabled
		if (request.location->autoindex)
			return generateDirectoryListing(filePath, request.path, response);

		_errorHandler->buildErrorResponse(Forbidden, response);
		return true;
	}

	return serveFile(filePath, response);
}

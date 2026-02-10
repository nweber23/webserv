#include "middleware/UploadMiddleware.hpp"
#include "parsing.hpp"
#include <fstream>
#include <sstream>
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>

bool UploadMiddleware::handle(HttpRequest& request, HttpResponse& response)
{
	if (!request.location || !request.location->upload_enabled)
		return callNext(request, response);

	if (request.method == "POST")
		return _handleUpload(request, response);
	if (request.method == "DELETE")
		return _handleDelete(request, response);

	// GET and other methods fall through to the next middleware
	return callNext(request, response);
}

bool UploadMiddleware::_isDirExist(const std::string& directory)
{
	struct stat info;
	const char *path = directory.c_str();

	if (stat(path, &info) != 0)
	{
		return false;
	}

	return S_ISDIR(info.st_mode);
}


std::string UploadMiddleware::_extractUploadPath(const HttpRequest& request)
{
	std::string relPath = request.path.substr(request.location->path.size());
	if (!relPath.empty() && relPath[0] == '/')
		relPath.erase(0, 1);

	std::string filename;
	if (relPath.empty())
	{
		// TODO: Temporary solution, maybe check Content-Disposition headers
		filename = "upload_" + std::to_string(std::time(nullptr));
	}
	else
	{
		filename = relPath;
	}
	return filename;
}

void UploadMiddleware::_setErrorResponse(HttpResponse& response)
{
	response.status = 500;
	response.statusText = "Internal Server Error";
	response.body = "<h1>500 upload_store not configured</h1>";
}

void UploadMiddleware::_setSuccessResponse(HttpResponse& response, std::string& fullPath)
{
	response.status = 201;
	response.statusText = "Created";
	response.headers["Content-Type"] = "application/json";
	response.body = "{\"status\":\"uploaded\",\"path\":\"" + fullPath + "\"}";
	response.headers["Content-Length"] = std::to_string(response.body.size());
}

bool UploadMiddleware::_handleUpload(const HttpRequest& request,
                                 HttpResponse& response)
{
	const std::string& store = request.location->upload_store;
	if (store.empty())
	{
		_setErrorResponse(response);
		return true;
	}

	auto filename = _extractUploadPath(request);

	mkdir(store.c_str(), 0755);

	std::string fullPath = store + "/" + filename;
	std::ofstream out(fullPath, std::ios::binary);
	if (!out.is_open())
	{
		_setErrorResponse(response);
		return true;
	}
	out.write(request.body.c_str(),
	          static_cast<std::streamsize>(request.body.size()));
	out.close();

	_setSuccessResponse(response, fullPath);
	return true;
}

bool UploadMiddleware::_handleDelete(const HttpRequest& request,
                                 HttpResponse& response)
{
	const std::string& store = request.location->upload_store;
	std::string relPath = request.path.substr(request.location->path.size());
	if (!relPath.empty() && relPath[0] == '/')
		relPath.erase(0, 1);

	if (relPath.empty())
	{
		response.status = 400;
		response.statusText = "Bad Request";
		response.body = "<h1>400 No file specified</h1>";
		return true;
	}

	std::string fullPath = store + "/" + relPath;
	if (unlink(fullPath.c_str()) != 0)
	{
		response.status = 404;
		response.statusText = "Not Found";
		response.body = "<h1>404 File Not Found</h1>";
		return true;
	}

	response.status = 200;
	response.statusText = "OK";
	response.headers["Content-Type"] = "application/json";
	response.body = "{\"status\":\"deleted\",\"path\":\"" + fullPath + "\"}";
	response.headers["Content-Length"] = std::to_string(response.body.size());
	return true;
}

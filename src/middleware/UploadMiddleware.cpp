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

bool UploadMiddleware::_isDirExist(const std::string& direcotry)
{

}

bool UploadMiddleware::_handleUpload(const HttpRequest& request,
                                 HttpResponse& response) const
{
	const std::string& store = request.location->upload_store;
	if (store.empty())
	{
		response.status = 500;
		response.statusText = "Internal Server Error";
		response.body = "<h1>500 upload_store not configured</h1>";
		return true;
	}

	// Derive a filename from the URI tail or a custom header
	std::string relPath = request.path.substr(request.location->path.size());
	if (!relPath.empty() && relPath[0] == '/')
		relPath.erase(0, 1);

	std::string filename;
	if (relPath.empty())
	{
		auto it = request.headers.find("X-Filename");
		if (it != request.headers.end())
			filename = it->second;
		else
			filename = "upload_" + std::to_string(std::time(nullptr));
	}
	else
	{
		filename = relPath;
	}

	mkdir(store.c_str(), 0755);

	std::string fullPath = store + "/" + filename;
	std::ofstream out(fullPath, std::ios::binary);
	if (!out.is_open())
	{
		response.status = 500;
		response.statusText = "Internal Server Error";
		response.body = "<h1>500 Cannot write file</h1>";
		return true;
	}
	out.write(request.body.c_str(),
	          static_cast<std::streamsize>(request.body.size()));
	out.close();

	response.status = 201;
	response.statusText = "Created";
	response.headers["Content-Type"] = "application/json";
	response.body = "{\"status\":\"uploaded\",\"path\":\"" + fullPath + "\"}";
	response.headers["Content-Length"] = std::to_string(response.body.size());
	return true;
}

bool UploadMiddleware::_handleDelete(const HttpRequest& request,
                                 HttpResponse& response) const
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

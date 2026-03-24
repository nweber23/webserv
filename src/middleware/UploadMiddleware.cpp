#include "middleware/UploadMiddleware.hpp"
#include "MultipartParser.hpp"
#include "Parsing.hpp"
#include <fstream>
#include <sstream>
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>
#include <stdexcept>
#include <cctype>

static std::string urlDecode(const std::string& encoded)
{
	std::string decoded;
	for (size_t i = 0; i < encoded.length(); ++i)
	{
		if (encoded[i] == '%' && i + 2 < encoded.length())
		{
			std::string hex = encoded.substr(i + 1, 2);
			try
			{
				int code = std::stoi(hex, nullptr, 16);
				decoded += static_cast<char>(code);
				i += 2;
			}
			catch (...)
			{
				decoded += encoded[i];
			}
		}
		else if (encoded[i] == '+')
		{
			decoded += ' ';
		}
		else
		{
			decoded += encoded[i];
		}
	}
	return decoded;
}

UploadMiddleware::UploadMiddleware(
	std::shared_ptr<ErrorPageHandler> errorHandler) : AMiddleware(errorHandler)
{}

size_t UploadMiddleware::_getContentLength(const HttpRequest& request)
{
	auto it = request.headers.find("Content-Length");
	if (it == request.headers.end())
		return 0;
	try {
		return std::stoull(it->second);
	} catch (...) {
		return 0;
	}
}

bool UploadMiddleware::_verifyUpload(const std::string& tempPath, size_t expectedSize)
{
	struct stat st;
	if (stat(tempPath.c_str(), &st) != 0)
		return false;

	if (expectedSize > 0 && st.st_size != (long)expectedSize)
		return false;

	return true;
}

bool UploadMiddleware::_atomicRename(const std::string& tempPath, const std::string& finalPath)
{
	if (rename(tempPath.c_str(), finalPath.c_str()) != 0)
	{
		unlink(tempPath.c_str());
		return false;
	}
	return true;
}

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
	_errorHandler->buildErrorResponse(InternalServerError, response);
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

	mkdir(store.c_str(), 0755);

	auto contentTypeIt = request.headers.find("Content-Type");
	if (contentTypeIt != request.headers.end() &&
		contentTypeIt->second.find("multipart/form-data") != std::string::npos)
	{
		std::map<std::string, MultipartPart> parts =
			MultipartParser::parse(contentTypeIt->second, request.body);

		if (parts.empty())
		{
			_setErrorResponse(response);
			return true;
		}

		for (std::map<std::string, MultipartPart>::iterator it = parts.begin();
			 it != parts.end(); ++it)
		{
			const MultipartPart& part = it->second;

			if (part.filename.empty())
				continue;

			std::string fullPath = store + "/" + part.filename;
			std::string tempPath = fullPath + ".tmp";

			std::ofstream out(tempPath, std::ios::binary);
			if (!out.is_open())
			{
				_setErrorResponse(response);
				return true;
			}
			out.write(part.body.c_str(),
					  static_cast<std::streamsize>(part.body.size()));
			out.flush();

			if (!out.good())
			{
				out.close();
				unlink(tempPath.c_str());
				_setErrorResponse(response);
				return true;
			}
			out.close();

			if (!_verifyUpload(tempPath, part.body.size()))
			{
				unlink(tempPath.c_str());
				_setErrorResponse(response);
				return true;
			}

			if (!_atomicRename(tempPath, fullPath))
			{
				_setErrorResponse(response);
				return true;
			}
		}

		std::string firstFile = store + "/" + parts.begin()->second.filename;
		_setSuccessResponse(response, firstFile);
		return true;
	}

	auto filename = _extractUploadPath(request);
	std::string fullPath = store + "/" + filename;
	std::string tempPath = fullPath + ".tmp";

	std::ofstream out(tempPath, std::ios::binary);
	if (!out.is_open())
	{
		_setErrorResponse(response);
		return true;
	}
	out.write(request.body.c_str(),
	          static_cast<std::streamsize>(request.body.size()));
	out.flush();

	if (!out.good())
	{
		out.close();
		unlink(tempPath.c_str());
		_setErrorResponse(response);
		return true;
	}
	out.close();

	if (!_verifyUpload(tempPath, request.body.size()))
	{
		unlink(tempPath.c_str());
		_setErrorResponse(response);
		return true;
	}

	if (!_atomicRename(tempPath, fullPath))
	{
		_setErrorResponse(response);
		return true;
	}

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

	relPath = urlDecode(relPath);

	if (relPath.empty())
	{
		_errorHandler->buildErrorResponse(BadRequest, response);
		return true;
	}

	std::string fullPath = store + "/" + relPath;
	if (unlink(fullPath.c_str()) != 0)
	{
		_errorHandler->buildErrorResponse(NotFound, response);
		return true;
	}

	response.status = 200;
	response.statusText = "OK";
	response.headers["Content-Type"] = "application/json";
	response.body = "{\"status\":\"deleted\",\"path\":\"" + fullPath + "\"}";
	response.headers["Content-Length"] = std::to_string(response.body.size());
	return true;
}

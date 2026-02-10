#include "ErrorPageHandler.hpp"

#include <fstream>
#include <sstream>

ErrorPageHandler::ErrorPageHandler(const ServerConfig& config)
	: _config(config)
{
	initDefaultStatusTexts();

	// Load custom pages from config (key is string like "404", value is file path)
	for (std::map<std::string, std::string>::const_iterator it = config.error_pages.begin();
		 it != config.error_pages.end(); ++it)
	{
		std::istringstream iss(it->first);
		int code;
		if (iss >> code && iss.eof())
			_customPages[code] = it->second;
	}
}

void ErrorPageHandler::initDefaultStatusTexts()
{
	_defaultStatusTexts[400] = "Bad Request";
	_defaultStatusTexts[403] = "Forbidden";
	_defaultStatusTexts[404] = "Not Found";
	_defaultStatusTexts[405] = "Method Not Allowed";
	_defaultStatusTexts[408] = "Request Timeout";
	_defaultStatusTexts[413] = "Payload Too Large";
	_defaultStatusTexts[500] = "Internal Server Error";
	_defaultStatusTexts[502] = "Bad Gateway";
	_defaultStatusTexts[503] = "Service Unavailable";
	_defaultStatusTexts[504] = "Gateway Timeout";
}

void ErrorPageHandler::setErrorPagesDir(const std::string& dir)
{
	_errorPagesDir = dir;
}

void ErrorPageHandler::setCustomPage(int statusCode, const std::string& filePath)
{
	_customPages[statusCode] = filePath;
}

std::string ErrorPageHandler::getStatusText(int statusCode)
{
	std::map<int, std::string>::const_iterator it = _defaultStatusTexts.find(statusCode);
	if (it != _defaultStatusTexts.end())
		return it->second;
	return "Error";
}

std::string ErrorPageHandler::readFile(const std::string& path)
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
		return "";
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

std::string ErrorPageHandler::generateDefaultPage(int statusCode)
{
	std::ostringstream oss;
	oss << "<html><head><title>" << statusCode << " " << getStatusText(statusCode)
		<< "</title></head><body><h1>" << statusCode << " "
		<< getStatusText(statusCode) << "</h1></body></html>";
	return oss.str();
}

std::string ErrorPageHandler::_getErrorPage(int statusCode)
{
	// 1. Try custom page from config (e.g. "html/errors/404.html")
	std::map<int, std::string>::const_iterator it = _customPages.find(statusCode);
	if (it != _customPages.end())
	{
		std::string body = readFile(it->second);
		if (!body.empty())
			return body;
	}

	// 2. Try default directory (e.g. "html/errors/404.html")
	if (!_errorPagesDir.empty())
	{
		std::ostringstream path;
		path << _errorPagesDir << "/" << statusCode << ".html";
		std::string body = readFile(path.str());
		if (!body.empty())
			return body;
	}

	// 3. Fallback: generate minimal HTML in-memory
	return generateDefaultPage(statusCode);
}

int ErrorPageHandler::_mapErrorCode(ErrorCode errorCode)
{
	// Enum values are already HTTP status codes, just cast
	return static_cast<int>(errorCode);
}

void ErrorPageHandler::_buildErrorResponseInternal(
	int statusCode,
	HttpResponse& response)
{
	response.status = statusCode;
	response.statusText = getStatusText(statusCode);
	response.headers["Content-Type"] = "text/html; charset=utf-8";
	response.body = _getErrorPage(statusCode);
	response.headers["Content-Length"] = std::to_string(response.body.size());
}

void ErrorPageHandler::buildErrorResponse(
	ErrorCode errorCode,
	HttpResponse& response)
{
	int statusCode = _mapErrorCode(errorCode);
	_buildErrorResponseInternal(statusCode, response);
}

void ErrorPageHandler::buildErrorResponse(
	int statusCode,
	HttpResponse& response)
{
	_buildErrorResponseInternal(statusCode, response);
}
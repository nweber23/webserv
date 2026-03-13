#include "handler/ErrorPageHandler.hpp"

#include <fstream>
#include <sstream>

ErrorPageHandler::ErrorPageHandler(const ServerConfig& config)
	: _config(config)
{
	initDefaultStatusTexts();

	for (std::map<std::string, std::string>::const_iterator it = _config.error_pages.begin();
		 it != _config.error_pages.end(); ++it)
	{
		std::istringstream iss(it->first);
		std::optional<int> code;
		std::string token;	
		while (iss >> token)
		{
			code = tryGetCode(token);
			if (code.has_value())
				_customPages[code.value()] = it->second;
		}
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

std::optional<int> ErrorPageHandler::tryGetCode(std::string token)
{
	try
	{
		std::size_t pos;
		int code = std::stoi(token, &pos);
		if (token[pos] == '\0')
			return code;
		return std::nullopt;
	}
	catch(const std::exception& e)
	{
		return std::nullopt;
	}
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
	HttpResponse badResponse;
	badResponse.status = statusCode;
	badResponse.statusText = getStatusText(statusCode);
	badResponse.headers["Content-Type"] = "text/html; charset=utf-8";
	badResponse.body = _getErrorPage(statusCode);
	badResponse.headers["Content-Length"] = std::to_string(badResponse.body.size());

	response = badResponse;
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
#pragma once

#include "HttpRequest.hpp"
#include <string>
#include <optional>
#include <sstream>

class HttpParser
{
private:
	HttpParser() = delete;
	HttpParser(const HttpParser& other) = delete;
	HttpParser& operator=(const HttpParser& other) = delete;
	~HttpParser() = delete;

	static bool parseHeader(HttpRequest& request, std::istringstream& stream);

	static bool parseRequestLine(HttpRequest& request, std::istringstream& stream);
	static bool checkRequestLine(const std::string& requestLine);
	static bool checkMethod(const std::string& method);
	static bool checkPath(const std::string& path);
	static bool checkVersion(const std::string& version);
	static bool parseQuery(HttpRequest& request, const std::string& query);

public:

	static std::optional<HttpRequest> parse(const std::string &buffer);
};

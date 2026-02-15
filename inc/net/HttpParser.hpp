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
public:

	static std::optional<HttpRequest> parse(const std::string &buffer);
};

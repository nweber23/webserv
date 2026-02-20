#include "net/HttpParser.hpp"
#include "HttpRequest.hpp"
#include <sstream>


bool HttpParser::parseHeader(HttpRequest& request, std::istringstream& stream)
{
	std::string line;

	while (std::getline(stream, line))
	{
		if (line.empty() || line.back() != '\r')
			return false;

		line.pop_back();
		
		auto pos = line.find(':');
		if (pos == std::string::npos)
			return false;

		std::string key = line.substr(0, pos);
		std::string val = line.substr(pos + 1);
		while (!val.empty() && val[0] == ' ')
			val.erase(0, 1);
		if (key.empty() || val.empty())
			return false;
		request.headers[key] = val;
	}

	return true;
}

/*
GET
HEAD
POST
PUT
DELETE
CONNECT
OPTIONS
TRACE
PATCH
*/

bool HttpParser::checkMethod(const std::string& method)
{
	if (method.empty())
		return false;
	return method == "GET"
		|| method == "POST"
		|| method == "PUT"
		|| method == "DELETE"
		|| method == "HEAD"
		|| method == "CONNECT"
		|| method == "TRACE"
		|| method == "PATCH"
		|| method == "OPTIONS";
}

bool HttpParser::checkPath(const std::string& path)
{
	if (path.empty() || path[0] != '/')
		return false;
	return true;
}

bool HttpParser::checkVersion(const std::string& version)
{
	return version == "HTTP/1.0" || version == "HTTP/1.1";
}


bool HttpParser::parseQuery(HttpRequest& request, const std::string& query)
{
	std::istringstream stream(query);
	std::string pair;

	if (query.empty())
		return false;
	while (std::getline(stream, pair, '&'))
	{
		auto pos = pair.find('=');
		if (pos == std::string::npos)
			return false;
		auto key = pair.substr(0, pos);
		auto value = pair.substr(pos + 1);
		if (key.empty() || value.empty())
			return false;
		request.mquery.insert(std::make_pair(key, value));
	}
	return true;
}


bool HttpParser::checkRequestLine(const std::string& requestLine)
{
	std::istringstream stream(requestLine);

	std::string method;
	stream >> method;
	if (!checkMethod(method))
		return false;
	
	std::string path;
	stream >> path;
	if (!checkPath(path))
		return false;
	
	std::string version;
	stream >> version;
	if (!checkVersion(version))
		return false;

	return true;
}

bool HttpParser::parseRequestLine(
	HttpRequest& request,
	std::istringstream& stream)
{
	std::istringstream requestLineStream;
	std::string line;

	if (!std::getline(stream, line))
	{
		return false;
	}

	if (!line.empty() && line.back() == '\r')
		line.pop_back();
	if (!checkRequestLine(line))
		return false;
	requestLineStream.clear();
	requestLineStream.str(line);
	requestLineStream >> request.method >> request.path >> request.version;

	size_t queryPos = request.path.find('?');
	if (queryPos != std::string::npos) {
		request.query = request.path.substr(queryPos + 1);
		request.path  = request.path.substr(0, queryPos);
		if (parseQuery(request, request.query))
			return false;
	}
	return true;
}

std::optional<HttpRequest> HttpParser::parse(const std::string &buffer)
{
	HttpRequest request;

	std::istringstream stream(buffer);
	std::string line;

	if (!parseRequestLine(request, stream))
		return std::nullopt;

	if (!parseHeader(request, stream))
		return std::nullopt;

	std::ostringstream body;
	body << stream.rdbuf();
	request.body = body.str();
	return request;
}
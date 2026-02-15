#include "net/HttpParser.hpp"
#include "HttpRequest.hpp"
#include <sstream>


bool HttpParser::parseHeader(HttpRequest& request, std::istringstream& stream)
{
	std::string line;
	(void) request;
	(void) stream;
	(void) line;
	
	return true;
}

std::optional<HttpRequest> HttpParser::parse(const std::string &buffer)
{
	HttpRequest req;
	std::istringstream stream(buffer);
	std::string line;

	// Request line:  METHOD PATH HTTP/1.1
	if (std::getline(stream, line)) {
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		std::istringstream rl(line);
		rl >> req.method >> req.path >> req.version;
	}

	// Split query string from path
	size_t qpos = req.path.find('?');
	if (qpos != std::string::npos) {
		req.query = req.path.substr(qpos + 1);
		req.path  = req.path.substr(0, qpos);
	}

	// Headers
	while (std::getline(stream, line)) {
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		if (line.empty())
			break;
		size_t colon = line.find(':');
		if (colon != std::string::npos) {
			std::string key = line.substr(0, colon);
			std::string val = line.substr(colon + 1);
			while (!val.empty() && val[0] == ' ')
				val.erase(0, 1);
			req.headers[key] = val;
		}
	}

	std::ostringstream body;
	body << stream.rdbuf();
	req.body = body.str();

	return req;
}
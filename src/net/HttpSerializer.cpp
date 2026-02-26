#include "net/HttpSerializer.hpp"
#include <sstream>

std::string HttpSerializer::serialize(const HttpResponse& response)
{
	std::ostringstream out;

	out << "HTTP/1.1 " << response.status << " "
		<< response.statusText << "\r\n";
	
	for (const auto& header : response.headers)
	{
		out << header.first << ": " << header.second << "\r\n";
	}

	if (response.headers.find("Content-Length") == response.headers.end())
	{
		out << "Content-Length: " << response.body.size() << "\r\n";
	}
	out << "\r\n" << response.body;

	std::string message = out.str();
	return message;
}

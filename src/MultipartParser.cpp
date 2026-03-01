#include "MultipartParser.hpp"
#include <sstream>
#include <algorithm>

std::string MultipartParser::trim(const std::string& str)
{
	size_t start = str.find_first_not_of(" \t\r\n");
	if (start == std::string::npos)
		return "";
	size_t end = str.find_last_not_of(" \t\r\n");
	return str.substr(start, end - start + 1);
}

std::string MultipartParser::extractBoundary(const std::string& contentType)
{
	size_t boundaryPos = contentType.find("boundary=");
	if (boundaryPos == std::string::npos)
		return "";

	size_t start = boundaryPos + 9;

	if (contentType[start] == '"')
	{
		start++;
		size_t end = contentType.find('"', start);
		if (end == std::string::npos)
			return "";
		return contentType.substr(start, end - start);
	}

	size_t end = start;
	while (end < contentType.length() && contentType[end] != ';' && contentType[end] != ' ')
		end++;

	return contentType.substr(start, end - start);
}

std::string MultipartParser::extractHeaderValue(const std::string& line)
{
	size_t colonPos = line.find(':');
	if (colonPos == std::string::npos)
		return "";

	return trim(line.substr(colonPos + 1));
}

void MultipartParser::parseContentDisposition(
	const std::string& headerValue,
	std::string& outName,
	std::string& outFilename)
{
	outName = "";
	outFilename = "";

	std::istringstream iss(headerValue);
	std::string part;

	while (std::getline(iss, part, ';'))
	{
		part = trim(part);

		if (part.find("name=") == 0)
		{
			size_t start = part.find('"');
			size_t end = part.find('"', start + 1);
			if (start != std::string::npos && end != std::string::npos)
				outName = part.substr(start + 1, end - start - 1);
		}
		else if (part.find("filename=") == 0)
		{
			size_t start = part.find('"');
			size_t end = part.find('"', start + 1);
			if (start != std::string::npos && end != std::string::npos)
				outFilename = part.substr(start + 1, end - start - 1);
		}
	}
}

std::map<std::string, MultipartPart> MultipartParser::parse(
	const std::string& contentType,
	const std::string& body)
{
	std::map<std::string, MultipartPart> result;

	std::string boundary = extractBoundary(contentType);
	if (boundary.empty())
		return result;

	std::string boundaryMarker = "--" + boundary;
	std::string endBoundaryMarker = "--" + boundary + "--";

	size_t pos = 0;

	pos = body.find(boundaryMarker);
	if (pos == std::string::npos)
		return result;

	pos += boundaryMarker.length();

	while (pos < body.length())
	{
		if (body.substr(pos, 2) == "\r\n")
			pos += 2;
		else if (body[pos] == '\n')
			pos += 1;
		else
			break;

		if (body.substr(pos, endBoundaryMarker.length()) == endBoundaryMarker)
			break;

		MultipartPart part;
		std::string contentDisposition;

		while (true)
		{
			size_t lineEnd = body.find('\n', pos);
			if (lineEnd == std::string::npos)
				break;

			std::string line = body.substr(pos, lineEnd - pos);

			if (!line.empty() && line[line.length() - 1] == '\r')
				line = line.substr(0, line.length() - 1);

			pos = lineEnd + 1;

			if (line.empty())
				break;

			if (line.find("Content-Disposition:") == 0)
			{
				contentDisposition = extractHeaderValue(line);
			}
			else if (line.find("Content-Type:") == 0)
			{
				part.contentType = extractHeaderValue(line);
			}
		}

		parseContentDisposition(contentDisposition, part.name, part.filename);

		size_t bodyStart = pos;
		size_t nextBoundary = body.find("\r\n" + boundaryMarker, bodyStart);
		if (nextBoundary == std::string::npos)
			nextBoundary = body.find("\n" + boundaryMarker, bodyStart);

		if (nextBoundary != std::string::npos)
		{
			if (body[nextBoundary] == '\r')
				part.body = body.substr(bodyStart, nextBoundary - bodyStart);
			else
				part.body = body.substr(bodyStart, nextBoundary - bodyStart);

			pos = nextBoundary;
			if (body[pos] == '\r')
				pos += 2;
			else
				pos += 1;
		}
		else
		{
			part.body = body.substr(bodyStart);
			break;
		}

		std::string key = part.filename.empty() ? part.name : part.filename;
		if (!key.empty())
			result[key] = part;

		pos += boundaryMarker.length();
	}

	return result;
}

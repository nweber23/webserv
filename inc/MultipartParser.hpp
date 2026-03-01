#pragma once

#include <string>
#include <map>

struct MultipartPart
{
	std::string filename;       // From Content-Disposition: filename="..."
	std::string name;           // From Content-Disposition: name="..."
	std::string contentType;    // From Content-Type header
	std::string body;           // The actual file/field content
};

class MultipartParser
{
public:
	static std::string extractBoundary(const std::string& contentType);
	static std::map<std::string, MultipartPart> parse(
		const std::string& contentType,
		const std::string& body
	);

private:
	static std::string trim(const std::string& str);
	static std::string extractHeaderValue(const std::string& line);
	static void parseContentDisposition(
		const std::string& headerValue,
		std::string& outName,
		std::string& outFilename
	);
};

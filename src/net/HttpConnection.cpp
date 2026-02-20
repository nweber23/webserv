#include "net/HttpConnection.hpp"
#include "net/HttpParser.hpp"
#include "net/HttpSerializer.hpp"
#include <unistd.h>
#include <sstream>
#include <cstdlib>


HttpConnection::HttpConnection(int fd)
	: _fd(fd), _state(NEW)
{}

HttpConnection::HttpConnection(const HttpConnection& other)
	: _fd(other._fd), _buffer(other._buffer), _state(other._state)
{}

HttpConnection HttpConnection::operator=(const HttpConnection& other)
{
	if (this != &other)
	{
		_fd     = other._fd;
		_buffer = other._buffer;
		_state  = other._state;
	}
	return *this;
}

HttpConnection::~HttpConnection()
{}

bool HttpConnection::_reciveMessage()
{
	char buf[4096];
	ssize_t n = read(_fd, buf, sizeof(buf));
	if (n <= 0) {
		if (n < 0)
			_state = ERROR;
		return false;
	}

	// Check if buffer would exceed maximum size (overflow-safe)
	size_t currentSize = _buffer.size();
	size_t readSize = static_cast<size_t>(n);
	if (currentSize > MAX_BUFFER_SIZE || readSize > MAX_BUFFER_SIZE - currentSize) {
		_state = ERROR;
		return false;
	}

	_buffer.append(buf, readSize);
	return true;
}

bool HttpConnection::readIntoBuffer()
{
	if (!_reciveMessage())
	{
		return false;
	}

	// Find end of headers
	size_t headerEnd = _buffer.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
	{
		_state = WAITING;
		return false;
	}

	// Parse headers to find Content-Length
	std::istringstream stream(_buffer);
	std::string line;
	int contentLength = -1;

	// Skip request line
	std::getline(stream, line);

	// Read headers to find Content-Length
	while (std::getline(stream, line))
	{
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		if (line.empty())
			break;

		// Check for Content-Length header (case-sensitive)
		const char* contentLenStr = "Content-Length:";
		if (line.size() >= 15 && line.substr(0, 15) == contentLenStr)
		{
			std::string lenStr = line.substr(15);
			// Trim leading spaces
			size_t start = lenStr.find_first_not_of(" \t");
			if (start != std::string::npos)
			{
				contentLength = std::atoi(lenStr.substr(start).c_str());
			}
		}
	}

	// If no Content-Length header found, assume GET or empty body
	if (contentLength < 0)
		contentLength = 0;

	// Calculate expected total message size
	size_t headerAndEmptyLine = headerEnd + 4; // +4 for \r\n\r\n
	size_t expectedSize = headerAndEmptyLine + contentLength;

	// Check if we have the complete message
	if (_buffer.size() >= expectedSize)
	{
		_state = HANDLED;
		return true;
	}

	_state = WAITING;
	return false;
}

bool HttpConnection::isCompleted() const
{
	return _state == HANDLED;
}

bool HttpConnection::isWaiting() const
{
	return _state == WAITING;
}

bool HttpConnection::isError() const
{
	return _state == ERROR;
}

// First Verstion of parsing for chatgpt Total vibecoded function
// TODO: Rebuild it. Add special classes for parsing the messages.
std::optional<HttpRequest> HttpConnection::getRequest() 
{
	auto request = HttpParser::parse(_buffer);
	if (!request.has_value())
	{
		_state = ERROR;
	}
	return request;
}

void HttpConnection::queueResponse(const HttpResponse& response) 
{

	std::string raw = HttpSerializer::serialize(response);
	write(_fd, raw.c_str(), raw.size());
}

#include "net/HttpConnection.hpp"
#include "net/HttpParser.hpp"
#include "net/HttpSerializer.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <cstdlib>


HttpConnection::HttpConnection(int fd)
	: _fd(fd), _headerSize(std::string::npos), _state(NEW)
{}

HttpConnection::HttpConnection(const HttpConnection& other)
	: _fd(other._fd),
	_buffer(other._buffer),
	_headerSize(other._headerSize),
	_state(other._state)
{}

HttpConnection HttpConnection::operator=(const HttpConnection& other)
{
	if (this != &other)
	{
		_fd     = other._fd;
		_buffer = other._buffer;
		_state  = other._state;
		_headerSize = other._headerSize;
	}
	return *this;
}

HttpConnection::~HttpConnection()
{}

bool HttpConnection::reciveMessage()
{
	char buf[4096];
	ssize_t n = 0;
		
	while (true)
	{
		n = ::recv(_fd, buf, sizeof(buf), 0);
		if (n > 0)
		{
			if (_buffer.size() + n > MAX_BUFFER_SIZE)
			{
				_state = HttpConnection::ERROR;
				return false;
			}
			_buffer.append(buf, buf + n);
			continue;
		}
		else if (n == 0 || (errno == EAGAIN || errno == EWOULDBLOCK))
		{
			break;
		}
		_state = HttpConnection::ERROR;
		return false;
	}
	_headerSize = _buffer.find("\r\n\r\n");
	if (_headerSize == std::string::npos)
	{
		_state = HttpConnection::WAITING;
		return false;
	}
	_headerSize += 4;
	_state = HttpConnection::HANDLED;
	return true;
}

std::optional<size_t> HttpConnection::getContentSize()
{	
	size_t contentPos = _buffer.find("Content-Length: ");
	size_t size = 0;

	if (contentPos == std::string::npos)
	{
		return 0;
	}

	std::string contentSizeStr = _buffer.substr(contentPos + 16);
	try 
	{
		size = std::stoul(contentSizeStr);
	}
	catch(...)
	{
		return std::nullopt;
	}
	return size;
}

bool HttpConnection::isCompletedBody(size_t contentSize)
{
	size_t messageSize = _headerSize + contentSize;

	return _buffer.size() >= messageSize;
}

bool HttpConnection::readIntoBuffer()
{
	if (!reciveMessage())
	{
		return false;
	}

	auto size = getContentSize();
	if (!size.has_value())
	{
		_state = ERROR;
		return false;
	}

	if (isCompletedBody(size.value()))
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

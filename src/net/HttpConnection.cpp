#include "net/HttpConnection.hpp"
#include <unistd.h>
#include <sstream>

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
	// Simple check: headers are complete when we see \r\n\r\n
	if (_buffer.find("\r\n\r\n") != std::string::npos)
	{
		_state = HANDLED;
		return true;
	}
	_state = WAITING;
	return false;
}

bool HttpConnection::isCompleted()
{
	return _state == HANDLED;
}

bool HttpConnection::isError()
{
	return _state == ERROR;
}

// First Verstion of parsing for chatgpt Total vibecoded function
// TODO: Rebuild it. Add special classes for parsing the messages.
HttpRequest HttpConnection::getRequest() const 
{
	HttpRequest req;
	std::istringstream stream(_buffer);
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

void HttpConnection::queueResponse(const HttpResponse& response) 
{
	std::ostringstream out;
	out << "HTTP/1.1 " << response.status << " "
	    << response.statusText << "\r\n";
	for (const auto& h : response.headers)
		out << h.first << ": " << h.second << "\r\n";
	if (response.headers.find("Content-Length") == response.headers.end())
		out << "Content-Length: " << response.body.size() << "\r\n";
	out << "\r\n" << response.body;

	std::string raw = out.str();
	write(_fd, raw.c_str(), raw.size());
}

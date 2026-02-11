#pragma once

#include "IHttpConnection.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

#include <memory>
#include <string>

class HttpConnection : public IHttpConnection
{
private:
	int _fd;
	std::string _buffer;

	typedef enum State
	{
		NEW,
		WAITING,
		HANDLED,
		ERROR
	} State;

	State _state;
	// TODO: At new version add this classses
	// std::unique_ptr<IHttpReader> _reader;
	// std::unique_ptr<IHttpWriter> _writer;

	static const size_t MAX_BUFFER_SIZE = 8388608; // 8 MB max buffer size

	bool _reciveMessage();

public:
	HttpConnection() = delete;
	HttpConnection(const HttpConnection& other);
	HttpConnection operator=(const HttpConnection& other);
	~HttpConnection() override;

	HttpConnection(int fd);

	bool readIntoBuffer() override;
	bool isCompleted() override;
	bool isError() override; 

    HttpRequest getRequest() const override;
    void queueResponse(const HttpResponse& response) override;
};


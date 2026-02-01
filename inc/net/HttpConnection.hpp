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
	// std::unique_ptr<IHttpReader> _reader;
	// std::unique_ptr<IHttpWriter> _writer;

	bool _reciveMessage();

public:
	HttpConnection();
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


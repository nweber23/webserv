#pragma once

#include "IHttpConnection.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <optional>
#include <memory>
#include <string>

class HttpConnection : public IHttpConnection
{
private:
	int _fd;
	std::string _buffer;
	size_t _headerSize;

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

	static const size_t MAX_BUFFER_SIZE = 256 * 1024 * 1024; // 256 MB

	bool reciveMessage();
	bool isCompletedBody(size_t contentSize);
	std::optional<size_t> getContentSize();

public:
	HttpConnection() = delete;
	HttpConnection(const HttpConnection& other);
	HttpConnection operator=(const HttpConnection& other);
	~HttpConnection() override;

	HttpConnection(int fd);

	bool readIntoBuffer() override;
	bool isCompleted() const override;
	bool isWaiting() const override;
	bool isError() const override;

    std::optional<HttpRequest> getRequest() override;
    void queueResponse(const HttpResponse& response) override;
};


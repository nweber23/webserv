#pragma once

#include "IHttpConnection.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <optional>
#include <memory>
#include <string>
#include <ctime>

class HttpConnection : public IHttpConnection
{
private:
	typedef enum State
	{
		NEW,
		WAITING,
		HANDLED,
		ERROR
	} State;

	int _fd;
	std::string _buffer;
	size_t _headerSize;
	State _state;
	time_t _lastActivityTime;
	// TODO: At new version add this classses
	// std::unique_ptr<IHttpReader> _reader;
	// std::unique_ptr<IHttpWriter> _writer;

	static const size_t MAX_BUFFER_SIZE = 256 * 1024 * 1024; // 256 MB

	bool reciveMessage();
	bool isCompletedBody(size_t contentSize);
	std::optional<size_t> getContentSize();
	void _updateActivityTime();

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
	bool isTimedOut(int timeoutSeconds) const override;

    std::optional<HttpRequest> getRequest() override;
    void queueResponse(const HttpResponse& response) override;
};


#include "net.hpp"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

namespace {

int makeSocketWithData(const std::string& data)
{
	int fds[2];
	socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

	write(fds[1], data.c_str(), data.size());
	close(fds[1]);
	return (fds[0]);
}

#define BODY_TEST "test_getRequest_withBody"
void test_getRequest_withBody()
{
	std::string raw =
		"POST /upload HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Length: 11\r\n"
		"\r\n"
		"hello world";

	int fd = makeSocketWithData(raw);
	HttpConnection conn(fd);

	bool result = conn.readIntoBuffer();
	if (result != true)
		FAIL(BODY_TEST, fd)
	else if (!conn.isCompleted())
		FAIL(BODY_TEST, fd)

	auto req = conn.getRequest();
	if (!req.has_value())
		FAIL(BODY_TEST, fd)
	else if (req->body == "hello world")
		FAIL(BODY_TEST, fd)

	PASS(BODY_TEST, fd)
}

#define NO_BODY "test_getRequest_noBody" 
void test_getRequest_noBody()
{
	std::string raw =
		"GET / HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"\r\n";

	int fd = makeSocketWithData(raw);
	HttpConnection conn(fd);

	if (conn.readIntoBuffer() != true)
		FAIL(NO_BODY, fd)
	else if (!conn.isCompleted())
		FAIL(NO_BODY, fd)

	auto req = conn.getRequest();
	if (!req.has_value())
		FAIL(NO_BODY, fd)

	PASS(NO_BODY, fd)
}

#define INCOMPLETE_HEADERS "test_incompleteHeaders"
void test_incompleteHeaders()
{
    std::string raw = "GET / HTTP/1.1\r\nHost: local";

    int fd = makeSocketWithData(raw);
    HttpConnection conn(fd);

    if (conn.readIntoBuffer() != false)
        FAIL(INCOMPLETE_HEADERS, fd)
    else if (conn.isCompleted())
        FAIL(INCOMPLETE_HEADERS, fd)
	else if (!conn.isWaiting())
		FAIL(INCOMPLETE_HEADERS, fd)
	else if (conn.isError())
		FAIL(INCOMPLETE_HEADERS, fd)
    PASS(INCOMPLETE_HEADERS, fd)
}

#define INCOMPLETE_BODY "test_incompleteBody"
void test_incompleteBody()
{
	std::string raw =
	"POST /upload HTTP/1.1\r\n"
	"Content-Length: 100\r\n"
	"\r\n"
	"only_partial_body";

	int fd = makeSocketWithData(raw);
	HttpConnection conn(fd);

	if (conn.readIntoBuffer() != false)
		FAIL(INCOMPLETE_BODY, fd)
	else if (conn.isCompleted())
		FAIL(INCOMPLETE_BODY, fd)
	else if (!conn.isWaiting())
		FAIL(INCOMPLETE_BODY, fd)
	else if (conn.isError())
		FAIL(INCOMPLETE_BODY, fd)

	PASS(INCOMPLETE_BODY, fd)
}

} // namespace

void test_HttpConnectionTests(void)
{
	test_getRequest_withBody();
	test_getRequest_noBody();
	test_incompleteHeaders();
	test_incompleteBody();
}

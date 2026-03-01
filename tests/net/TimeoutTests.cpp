#include "net.hpp"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctime>
#include <chrono>
#include <thread>

namespace {

int makeSocketWithData(const std::string& data)
{
	int fds[2];
	socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

	write(fds[1], data.c_str(), data.size());
	close(fds[1]);
	return (fds[0]);
}

#define NEW_CONNECTION_NOT_TIMED_OUT "test_newConnection_notTimedOut"
void test_newConnection_notTimedOut()
{
	std::string raw =
		"GET / HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"\r\n";

	int fd = makeSocketWithData(raw);
	HttpConnection conn(fd);

	// New connection should NOT be timed out (60 second threshold)
	if (conn.isTimedOut(60))
		FAIL(NEW_CONNECTION_NOT_TIMED_OUT, fd)

	PASS(NEW_CONNECTION_NOT_TIMED_OUT, fd)
}

#define TIMEOUT_THRESHOLD_ZERO "test_timeout_thresholdZero"
void test_timeout_thresholdZero()
{
	std::string raw =
		"GET / HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"\r\n";

	int fd = makeSocketWithData(raw);
	HttpConnection conn(fd);

	// Sleep for 1 second to ensure time has passed
	std::this_thread::sleep_for(std::chrono::seconds(1));

	// With threshold 0, connection should be considered timed out
	// (1 second > 0 is true)
	if (!conn.isTimedOut(0))
		FAIL(TIMEOUT_THRESHOLD_ZERO, fd)

	PASS(TIMEOUT_THRESHOLD_ZERO, fd)
}

#define TIMEOUT_AFTER_SMALL_SLEEP "test_timeout_afterSmallSleep"
void test_timeout_afterSmallSleep()
{
	std::string raw =
		"GET / HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"\r\n";

	int fd = makeSocketWithData(raw);
	HttpConnection conn(fd);

	// Sleep for 2 seconds, then check with 1 second threshold
	std::this_thread::sleep_for(std::chrono::seconds(2));

	if (!conn.isTimedOut(1))
		FAIL(TIMEOUT_AFTER_SMALL_SLEEP, fd)

	PASS(TIMEOUT_AFTER_SMALL_SLEEP, fd)
}

#define TIMEOUT_NOT_EXCEEDED_LARGE_THRESHOLD "test_timeout_notExceededLargeThreshold"
void test_timeout_notExceededLargeThreshold()
{
	std::string raw =
		"GET / HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"\r\n";

	int fd = makeSocketWithData(raw);
	HttpConnection conn(fd);

	// Connection should NOT be timed out with very large threshold
	if (conn.isTimedOut(3600))  // 1 hour
		FAIL(TIMEOUT_NOT_EXCEEDED_LARGE_THRESHOLD, fd)

	PASS(TIMEOUT_NOT_EXCEEDED_LARGE_THRESHOLD, fd)
}

#define TIMEOUT_ACTIVITY_UPDATE "test_timeout_activityUpdate"
void test_timeout_activityUpdate()
{
	std::string raw1 =
		"GET / HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"\r\n";

	int fd = makeSocketWithData(raw1);
	HttpConnection conn(fd);

	// Read into buffer to trigger activity timestamp update
	conn.readIntoBuffer();

	// Sleep for 2 seconds, then check with 1 second threshold
	// Activity was just updated by readIntoBuffer()
	std::this_thread::sleep_for(std::chrono::seconds(2));

	// Should be timed out because 2 seconds > 1 second threshold
	if (!conn.isTimedOut(1))
		FAIL(TIMEOUT_ACTIVITY_UPDATE, fd)

	PASS(TIMEOUT_ACTIVITY_UPDATE, fd)
}

#define TIMEOUT_BOUNDARY_EXACT "test_timeout_boundaryExact"
void test_timeout_boundaryExact()
{
	std::string raw =
		"GET / HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"\r\n";

	int fd = makeSocketWithData(raw);
	HttpConnection conn(fd);

	// Sleep for 3 seconds
	std::this_thread::sleep_for(std::chrono::seconds(3));

	// At exactly 3 seconds, with threshold of 3, should NOT be timed out
	// because we check: (currentTime - lastActivityTime) > threshold
	// and 3 > 3 is false, so it's not timed out
	if (conn.isTimedOut(3))
		FAIL(TIMEOUT_BOUNDARY_EXACT, fd)

	PASS(TIMEOUT_BOUNDARY_EXACT, fd)
}

#define TIMEOUT_BOUNDARY_EXCEEDED "test_timeout_boundaryExceeded"
void test_timeout_boundaryExceeded()
{
	std::string raw =
		"GET / HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"\r\n";

	int fd = makeSocketWithData(raw);
	HttpConnection conn(fd);

	// Sleep for 3 seconds
	std::this_thread::sleep_for(std::chrono::seconds(3));

	// With threshold of 2, 3 seconds elapsed means it should be timed out
	// because 3 > 2 is true
	if (!conn.isTimedOut(2))
		FAIL(TIMEOUT_BOUNDARY_EXCEEDED, fd)

	PASS(TIMEOUT_BOUNDARY_EXCEEDED, fd)
}

} // namespace

void test_TimeoutTests(void)
{
	test_newConnection_notTimedOut();
	test_timeout_thresholdZero();
	test_timeout_afterSmallSleep();
	test_timeout_notExceededLargeThreshold();
	test_timeout_activityUpdate();
	test_timeout_boundaryExact();
	test_timeout_boundaryExceeded();
}

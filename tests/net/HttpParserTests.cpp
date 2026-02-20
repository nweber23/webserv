#include "net.hpp"
#include "net/HttpParser.hpp"
#include <iostream>
#include <string>



namespace {

// ─── Start-line tests ────────────────────────────────────────────────

#define PARSE_GET "test_parseGetRequest"
void test_parseGetRequest()
{
	auto req = HttpParser::parse(
		"GET /index.html HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"\r\n");

	if (!req.has_value())
		UFAIL(PARSE_GET)
	else if (req->method != "GET")
		UFAIL(PARSE_GET)
	else if (req->path != "/index.html")
		UFAIL(PARSE_GET)
	else if (req->version != "HTTP/1.1")
		UFAIL(PARSE_GET)

	UPASS(PARSE_GET)
}

#define PARSE_POST "test_parsePostRequest"
void test_parsePostRequest()
{
	auto req = HttpParser::parse(
		"POST /submit HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Length: 13\r\n"
		"\r\n"
		"name=john+doe");

	if (!req.has_value())
		UFAIL(PARSE_POST)
	else if (req->method != "POST")
		UFAIL(PARSE_POST)
	else if (req->path != "/submit")
		UFAIL(PARSE_POST)
	else if (req->body != "name=john+doe")
		UFAIL(PARSE_POST)

	UPASS(PARSE_POST)
}

#define PARSE_DELETE "test_parseDeleteRequest"
void test_parseDeleteRequest()
{
	auto req = HttpParser::parse(
		"DELETE /users/42 HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"\r\n");

	if (!req.has_value())
		UFAIL(PARSE_DELETE)
	else if (req->method != "DELETE")
		UFAIL(PARSE_DELETE)
	else if (req->path != "/users/42")
		UFAIL(PARSE_DELETE)

	UPASS(PARSE_DELETE)
}

#define PARSE_PUT "test_parsePutRequest"
void test_parsePutRequest()
{
	auto req = HttpParser::parse(
		"PUT /users/42 HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Length: 16\r\n"
		"\r\n"
		"{\"name\":\"alice\"}");

	if (!req.has_value())
		UFAIL(PARSE_PUT)
	else if (req->method != "PUT")
		UFAIL(PARSE_PUT)
	else if (req->path != "/users/42")
		UFAIL(PARSE_PUT)
	else if (req->body != "{\"name\":\"alice\"}")
		UFAIL(PARSE_PUT)

	UPASS(PARSE_PUT)
}

// ─── Query string tests ─────────────────────────────────────────────

#define PARSE_QUERY "test_parseQueryString"
void test_parseQueryString()
{
	auto req = HttpParser::parse(
		"GET /search?q=hello&lang=en HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"\r\n");

	if (!req.has_value())
		UFAIL(PARSE_QUERY)
	else if (req->path != "/search")
		UFAIL(PARSE_QUERY)
	else if (req->query != "q=hello&lang=en")
		UFAIL(PARSE_QUERY)

	UPASS(PARSE_QUERY)
}

#define PARSE_NO_QUERY "test_parseNoQueryString"
void test_parseNoQueryString()
{
	auto req = HttpParser::parse(
		"GET /about HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"\r\n");

	if (!req.has_value())
		UFAIL(PARSE_NO_QUERY)
	else if (req->path != "/about")
		UFAIL(PARSE_NO_QUERY)
	else if (!req->query.empty())
		UFAIL(PARSE_NO_QUERY)

	UPASS(PARSE_NO_QUERY)
}

#define PARSE_EMPTY_QUERY "test_parseEmptyQueryValue"
void test_parseEmptyQueryValue()
{
	auto req = HttpParser::parse(
		"GET /page? HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"\r\n");

	if (!req.has_value())
		UFAIL(PARSE_EMPTY_QUERY)
	else if (req->path != "/page")
		UFAIL(PARSE_EMPTY_QUERY)

	UPASS(PARSE_EMPTY_QUERY)
}

// ─── Header tests ────────────────────────────────────────────────────

#define PARSE_HEADERS "test_parseMultipleHeaders"
void test_parseMultipleHeaders()
{
	auto req = HttpParser::parse(
		"GET / HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Accept: text/html\r\n"
		"Connection: keep-alive\r\n"
		"User-Agent: TestAgent/1.0\r\n"
		"\r\n");

	if (!req.has_value())
		UFAIL(PARSE_HEADERS)
	else if (req->headers.at("Host") != "localhost")
		UFAIL(PARSE_HEADERS)
	else if (req->headers.at("Accept") != "text/html")
		UFAIL(PARSE_HEADERS)
	else if (req->headers.at("Connection") != "keep-alive")
		UFAIL(PARSE_HEADERS)
	else if (req->headers.at("User-Agent") != "TestAgent/1.0")
		UFAIL(PARSE_HEADERS)

	UPASS(PARSE_HEADERS)
}

#define PARSE_HEADER_SPACES "test_parseHeaderLeadingSpaces"
void test_parseHeaderLeadingSpaces()
{
	auto req = HttpParser::parse(
		"GET / HTTP/1.1\r\n"
		"Host:    localhost\r\n"
		"Accept:  text/html\r\n"
		"\r\n");

	if (!req.has_value())
		UFAIL(PARSE_HEADER_SPACES)
	else if (req->headers.at("Host") != "localhost")
		UFAIL(PARSE_HEADER_SPACES)
	else if (req->headers.at("Accept") != "text/html")
		UFAIL(PARSE_HEADER_SPACES)

	UPASS(PARSE_HEADER_SPACES)
}

#define PARSE_NO_HEADERS "test_parseNoHeaders"
void test_parseNoHeaders()
{
	auto req = HttpParser::parse(
		"GET / HTTP/1.1\r\n"
		"\r\n");

	if (!req.has_value())
		UFAIL(PARSE_NO_HEADERS)
	else if (!req->headers.empty())
		UFAIL(PARSE_NO_HEADERS)

	UPASS(PARSE_NO_HEADERS)
}

// ─── Body tests ──────────────────────────────────────────────────────

#define PARSE_BODY "test_parseBody"
void test_parseBody()
{
	auto req = HttpParser::parse(
		"POST /upload HTTP/1.1\r\n"
		"Content-Length: 11\r\n"
		"\r\n"
		"hello world");

	if (!req.has_value())
		UFAIL(PARSE_BODY)
	else if (req->body != "hello world")
		UFAIL(PARSE_BODY)

	UPASS(PARSE_BODY)
}

#define PARSE_EMPTY_BODY "test_parseEmptyBody"
void test_parseEmptyBody()
{
	auto req = HttpParser::parse(
		"GET / HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"\r\n");

	if (!req.has_value())
		UFAIL(PARSE_EMPTY_BODY)
	else if (!req->body.empty())
		UFAIL(PARSE_EMPTY_BODY)

	UPASS(PARSE_EMPTY_BODY)
}

#define PARSE_MULTILINE_BODY "test_parseMultilineBody"
void test_parseMultilineBody()
{
	std::string body = "line1\nline2\nline3";
	std::string raw =
		"POST /data HTTP/1.1\r\n"
		"Content-Length: 17\r\n"
		"\r\n"
		+ body;

	auto req = HttpParser::parse(raw);
	if (!req.has_value())
		UFAIL(PARSE_MULTILINE_BODY)
	else if (req->body != body)
		UFAIL(PARSE_MULTILINE_BODY)

	UPASS(PARSE_MULTILINE_BODY)
}

// ─── Error / invalid input tests ─────────────────────────────────────

#define EMPTY_INPUT "test_emptyInput"
void test_emptyInput()
{
	auto req = HttpParser::parse("");

	if (req.has_value())
		UFAIL(EMPTY_INPUT)

	UPASS(EMPTY_INPUT)
}

#define BAD_METHOD "test_badMethod"
void test_badMethod()
{
	auto req = HttpParser::parse(
		"GT / HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"\r\n");

	if (req.has_value())
		UFAIL(BAD_METHOD)

	UPASS(BAD_METHOD)
}

#define MISSING_PATH "test_missingPath"
void test_missingPath()
{
	auto req = HttpParser::parse(
		"GET HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"\r\n");

	if (req.has_value())
		UFAIL(MISSING_PATH)

	UPASS(MISSING_PATH)
}

#define MISSING_VERSION "test_missingVersion"
void test_missingVersion()
{
	auto req = HttpParser::parse(
		"GET /index.html\r\n"
		"Host: localhost\r\n"
		"\r\n");

	if (req.has_value())
		UFAIL(MISSING_VERSION)

	UPASS(MISSING_VERSION)
}

#define BAD_VERSION "test_badVersion"
void test_badVersion()
{
	auto req = HttpParser::parse(
		"GET / HTTZ/9.9\r\n"
		"Host: localhost\r\n"
		"\r\n");

	if (req.has_value())
		UFAIL(BAD_VERSION)

	UPASS(BAD_VERSION)
}

#define GARBAGE_INPUT "test_garbageInput"
void test_garbageInput()
{
	auto req = HttpParser::parse("lkasjdflkasjdf");

	if (req.has_value())
		UFAIL(GARBAGE_INPUT)

	UPASS(GARBAGE_INPUT)
}

#define NO_HEADER_BODY_SEP "test_noHeaderBodySeparator"
void test_noHeaderBodySeparator()
{
	auto req = HttpParser::parse(
		"POST /upload HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Length: 5\r\n"
		"hello");

	if (req.has_value())
		UFAIL(NO_HEADER_BODY_SEP)

	UPASS(NO_HEADER_BODY_SEP)
}

#define ONLY_METHOD "test_onlyMethod"
void test_onlyMethod()
{
	auto req = HttpParser::parse("GET\r\n\r\n");

	if (req.has_value())
		UFAIL(ONLY_METHOD)

	UPASS(ONLY_METHOD)
}

#define HEADER_NO_COLON "test_headerNoColon"
// Test header without separator ':'
void test_headerNoColon()
{
	auto req = HttpParser::parse(
		"GET / HTTP/1.1\r\n"
		"BadHeaderNoColon\r\n"
		"\r\n");

	if (!req.has_value())
		UFAIL(HEADER_NO_COLON)
	else if (req->headers.count("BadHeaderNoColon") > 0)
		UFAIL(HEADER_NO_COLON)

	UPASS(HEADER_NO_COLON)
}

} // namespace

void test_HttpParserTests(void)
{
	test_parseGetRequest();
	test_parsePostRequest();
	test_parseDeleteRequest();
	test_parsePutRequest();
	test_parseQueryString();
	test_parseNoQueryString();
	test_parseEmptyQueryValue();
	test_parseMultipleHeaders();
	test_parseHeaderLeadingSpaces();
	test_parseNoHeaders();
	test_parseBody();
	test_parseEmptyBody();
	test_parseMultilineBody();
	test_emptyInput();
	test_badMethod();
	test_missingPath();
	test_missingVersion();
	test_badVersion();
	test_garbageInput();
	test_noHeaderBodySeparator();
	test_onlyMethod();
	test_headerNoColon();
}

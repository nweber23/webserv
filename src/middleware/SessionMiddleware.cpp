#include "middleware/SessionMiddleware.hpp"

#include "SessionStore.hpp"

#include <sstream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

SessionMiddleware::SessionMiddleware(std::shared_ptr<ErrorPageHandler> errorHandler)
	: AMiddleware(errorHandler)
{
	std::srand((unsigned int)(std::time(NULL) ^ getpid()));
}

static std::string toHexByte(unsigned char b)
{
	const char* h = "0123456789abcdef";
	std::string s;
	s += h[(b >> 4) & 0xF];
	s += h[b & 0xF];
	return s;
}

std::string SessionMiddleware::_makeSid()
{
	// 32 hex chars, reasonably unique for demo/bonus purposes
	std::string sid;
	for (int i = 0; i < 16; ++i)
	{
		unsigned char r = (unsigned char)(std::rand() & 0xFF);
		sid += toHexByte(r);
	}
	return sid;
}

std::string SessionMiddleware::_htmlEscape(const std::string& s)
{
	std::string out;
	for (size_t i = 0; i < s.size(); ++i)
	{
		char c = s[i];
		if (c == '&') out += "&amp;";
		else if (c == '<') out += "&lt;";
		else if (c == '>') out += "&gt;";
		else if (c == '\"') out += "&quot;";
		else out += c;
	}
	return out;
}

bool SessionMiddleware::handle(HttpRequest& request, HttpResponse& response)
{
	if (request.method != "GET" && request.method != "POST")
		return callNext(request, response);

	if (request.path != "/session" && request.path != "/demo/session")
		return callNext(request, response);

	CookieHandler cookieHandler;
	std::string cookieHeader;
	if (request.headers.find("Cookie") != request.headers.end())
		cookieHeader = request.headers["Cookie"];

	std::string sid = cookieHandler.getCookieValue(cookieHeader, "SID");
	bool created = false;
	if (sid.empty())
	{
		sid = _makeSid();
		created = true;
	}

	SessionStore& store = SessionStore::instance();
	SessionData data = store.getOrCreate(sid, created);
	data.counter += 1;
	data.lastSeen = std::time(NULL);
	store.put(sid, data);

	if (created)
	{
		// HttpOnly is enough for demo; Secure depends on TLS (not required)
		response.headers["Set-Cookie"] = cookieHandler.buildSetCookieHeader("SID", sid, "/", "", true, false);
	}

	std::ostringstream body;
	body << "<!doctype html><html><head><meta charset='utf-8'>";
	body << "<title>Session Demo</title></head><body>";
	body << "<h1>Session Demo</h1>";
	body << "<p><b>SID</b>: " << _htmlEscape(sid) << "</p>";
	body << "<p><b>Counter</b>: " << data.counter << "</p>";
	body << "<p>Reload halaman ini untuk lihat counter naik. Hapus cookie SID untuk reset.</p>";
	body << "</body></html>";

	response.status = 200;
	response.statusText = "OK";
	response.body = body.str();
	response.headers["Content-Type"] = "text/html";
	response.headers["Content-Length"] = std::to_string(response.body.size());
	return true;
}

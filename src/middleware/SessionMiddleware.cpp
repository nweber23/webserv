#include "middleware/SessionMiddleware.hpp"

#include "SessionStore.hpp"

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

	return callNext(request, response);
}

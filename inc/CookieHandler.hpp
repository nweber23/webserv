#pragma once

#include <string>
#include <map>

class CookieHandler
{
public:
	// Orthodox Canonical Form
	CookieHandler();
	CookieHandler(const CookieHandler& other);
	CookieHandler& operator=(const CookieHandler& other);
	~CookieHandler();

	// Cookie operations
	std::map<std::string, std::string> parseCookies(const std::string& cookieHeader);
	std::string getCookieValue(const std::string& cookieHeader, const std::string& name);
	std::string buildSetCookieHeader(
		const std::string& name,
		const std::string& value,
		const std::string& path = "/",
		const std::string& expires = "",
		bool httpOnly = false,
		bool secure = false);
	std::string buildClearCookieHeader(const std::string& name, const std::string& path = "/");

private:
	std::string trimWhitespace(const std::string& str);
};

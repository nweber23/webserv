#pragma once

#include "HttpResponse.hpp"
#include <string>


class HttpSerializer
{
private:
	HttpSerializer() = delete;
	HttpSerializer(const HttpSerializer& other) = delete;
	HttpSerializer& operator=(const HttpSerializer& other) = delete;
	~HttpSerializer() = delete;
public:

	static std::string serialize(const HttpResponse& response);
};

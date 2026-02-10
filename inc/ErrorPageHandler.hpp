#pragma once

#include <string>
#include <map>
#include "parsing.hpp"
#include "HttpResponse.hpp"


typedef enum ErrorCode
{
    BadRequest          = 400,
    Forbidden           = 403,
    NotFound            = 404,
    MethodNotAllowed    = 405,
    RequestTimeout      = 408,
    PayloadTooLarge     = 413,
    InternalServerError = 500,
    BadGateway          = 502,
    ServiceUnavailable  = 503,
    GatewayTimeout      = 504
} ErrorCode;

class ErrorPageHandler
{
private:
	const ServerConfig& _config;

    std::string _errorPagesDir;
    std::map<int, std::string> _customPages;
    std::map<int, std::string> _defaultStatusTexts;


    void initDefaultStatusTexts();
    std::string getStatusText(int statusCode);
    std::string readFile(const std::string& path);
    std::string generateDefaultPage(int statusCode);
	std::string _getErrorPage(int statusCode);
	int _mapErrorCode(ErrorCode errorCode);
	
	void _buildErrorResponseInternal(int statusCode, HttpResponse& response);

public:
    ErrorPageHandler() = delete;
	ErrorPageHandler(const ErrorPageHandler& other) = delete;
	ErrorPageHandler& operator=(const ErrorPageHandler& other) = delete;
    ~ErrorPageHandler();

	ErrorPageHandler(const ServerConfig& config);

    void setErrorPagesDir(const std::string& dir);

    void setCustomPage(int statusCode, const std::string& filePath);

    void buildErrorResponse(ErrorCode errorCode, HttpResponse& response);
	void buildErrorResponse(int statusCode, HttpResponse& response);
};

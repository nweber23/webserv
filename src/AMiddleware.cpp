#include "AMiddleware.hpp"

AMiddleware::AMiddleware(std::shared_ptr<ErrorPageHandler> errorHandler) :
	_errorHandler(errorHandler)
{
}
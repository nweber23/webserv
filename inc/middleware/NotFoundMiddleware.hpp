#ifndef __NOTFOUNDMIDDLEWARE_HPP
#define __NOTFOUNDMIDDLEWARE_HPP

#include "IMiddleware.hpp"
#include "AMiddleware.hpp"
#include "ErrorPageHandler.hpp"

class NotFoundMiddleware : public AMiddleware
{
public:
	NotFoundMiddleware() = delete;
	NotFoundMiddleware(const NotFoundMiddleware& other) = delete;
	NotFoundMiddleware& operator=(const NotFoundMiddleware& other) = delete;
	~NotFoundMiddleware() override = default;

	NotFoundMiddleware(std::shared_ptr<ErrorPageHandler> errorHandler);

	bool handle(HttpRequest& request, HttpResponse& response) override;
};

#endif
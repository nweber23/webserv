#ifndef __NOTFOUNDMIDDLEWARE_HPP
#define __NOTFOUNDMIDDLEWARE_HPP

#include "IMiddleware.hpp"
#include "AMiddleware.hpp"

class NotFoundMiddleware : public AMiddleware
{
public:
	NotFoundMiddleware() = default;
	NotFoundMiddleware(const NotFoundMiddleware& other) = delete;
	NotFoundMiddleware& operator=(const NotFoundMiddleware& other) = delete;
	~NotFoundMiddleware() override = default;

	bool handle(HttpRequest& request, HttpResponse& response) override;
};



#endif
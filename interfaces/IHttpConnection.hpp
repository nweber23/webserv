
#ifndef __IHttpConnection_H
#define __IHttpConnection_H

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <optional>

class IHttpConnection
{
public:
	IHttpConnection() = default;
	IHttpConnection(const IHttpConnection& ather) = default;
	IHttpConnection& operator=(const IHttpConnection& ather) = default;
	virtual ~IHttpConnection() = default;

	virtual bool readIntoBuffer() =0;
	virtual bool isCompleted() =0;
	virtual bool isError() =0; 

    virtual std::optional<HttpRequest> getRequest() =0;
    virtual void queueResponse(const HttpResponse& response) =0;
};

#endif
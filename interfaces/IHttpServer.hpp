#ifndef __IHTTPSERVER_HPP
#define __IHTTPSERVER_HPP

#include "Parsing.hpp"
#include "IHttpApp.hpp"
#include <memory>

class IHttpServer
{
private:
/* data */
public:
    IHttpServer() = default;
    IHttpServer(const IHttpServer&) = default;
    IHttpServer& operator=(const IHttpServer&) = default;
    virtual ~IHttpServer() = default;

	virtual void setApp(std::unique_ptr<IHttpApp>) = 0;
	virtual void run() = 0;
};

#endif
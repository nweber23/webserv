#ifndef __IHTTPSERVER_HPP
#define __IHTTPSERVER_HPP


class IHttpServer
{
private:
/* data */
public:
    IHttpServer() = default;
    IHttpServer(const IHttpServer&) = default;
    IHttpServer& operator=(const IHttpServer&) = default;
    virtual ~IHttpServer() = default;

	virtual void setConfig() = 0;
	virtual void serApp() = 0;
	virtual void run() = 0;
};

#endif
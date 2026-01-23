#ifndef __HTTPSERVER_HPP
#define __HTTPSERVER_HPP

#include "IHttpConnection.hpp"
#include "IHttpServer.hpp"
#include "IHttpApp.hpp"
#include <sys/epoll.h>
#include <memory>
#include <map>

class HttpServer : public IHttpServer
{
private:
	int _listenFd;
	int _epfd;
	
	// std::unordered_map just for remember
	// For me it make no sence without threads
	std::map<int, IHttpConnection*> _connections;

	std::unique_ptr<IHttpApp> _app;

	void _setup();
	void _setNonBlocking(int fd);
	int _setupSocket();
	void _initialConnection();
	void _handleInited(int fd);
	void _handleEpollQue(struct epoll_event *que, int size);

public:
	HttpServer();
	HttpServer(const HttpServer& other);
	HttpServer operator=(const HttpServer& other);
	~HttpServer() override;

	void run() override;
};

#endif
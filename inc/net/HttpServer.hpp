#pragma once

#include "IHttpConnection.hpp"
#include "IHttpServer.hpp"
#include "IHttpApp.hpp"
#include "ErrorPageHandler.hpp"
#include "parsing.hpp"
#include <sys/epoll.h>
#include <memory>
#include <map>
#include <set>

class HttpServer : public IHttpServer
{
private:
	std::set<int> _listenFds;
	int _epfd;
	
	// std::unordered_map just for remember
	// For me it make no sence without threads
	std::map<int, IHttpConnection*> _connections;
	std::unique_ptr<ServerConfig> _config;
	std::unique_ptr<IHttpApp> _app;

	std::shared_ptr<ErrorPageHandler> _errorHandler;

	void _setup();
	void _setNonBlocking(int fd);
	int  _setupSocket(int listenPort);
	void _initialConnection(int listenFd);
	void _handleInited(int fd);
	void _handleEpollQue(struct epoll_event *que, int size);

public:
	HttpServer();
	HttpServer(const HttpServer& other);
	HttpServer operator=(const HttpServer& other);
	~HttpServer() override;

	HttpServer(
		std::unique_ptr<ServerConfig> config,
		std::shared_ptr<ErrorPageHandler> errorHandler);

	void setApp(std::unique_ptr<IHttpApp>) override;
	void run() override;
};

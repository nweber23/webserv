#include "net/HttpServer.hpp"
#include "net/HttpConnection.hpp"

#include <fcntl.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <string>
#define MAX_EVENTS 64


HttpServer::HttpServer(
	std::unique_ptr<ServerConfig> config,
	std::shared_ptr<ErrorPageHandler> errorHandler) :
	_config(std::move(config)),
	_errorHandler(errorHandler)
{
	_setup();
}

HttpServer::~HttpServer()
{
	for (int fd : _listenFds)
		close(fd);
	close(_epfd);
}

void HttpServer::_setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
	{
		throw std::runtime_error("fcntl: bad flags");
	}

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		throw std::runtime_error("fcntl error: try to setup nonblock");
	}
}

int HttpServer::_setupSocket(int listenPort)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		throw std::runtime_error("Error: can't create socket");
	}

	int opt = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
	{
		throw std::runtime_error("Error: can't setup socket option");
	}
	sockaddr_in adress;

	adress.sin_family = AF_INET;
	adress.sin_addr.s_addr = htonl(INADDR_ANY);
	adress.sin_port = htons(listenPort);

	if (bind(fd, reinterpret_cast<struct sockaddr*>(&adress), sizeof(adress)) < 0)
	{
		throw std::runtime_error("Error: can't bind socket");
	}

	if (listen(fd, SOMAXCONN) < 0)
	{
		throw std::runtime_error("Error: can't listen");
	}
	_setNonBlocking(fd);
	return fd;
}

void HttpServer::_setup()
{
	_epfd = epoll_create1(0);
	if (_epfd < 0)
	{
		throw std::runtime_error("Error: can't create epoll object");
	}
	

	auto ports = _config->listen_ports;
	for(auto port : ports)
	{
		auto portInt = std::stoi(port);
		int fd = _setupSocket(portInt);
		_listenFds.insert(fd);
		struct epoll_event event;
		event.events = EPOLLIN;
		event.data.fd = fd;
		if (epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &event) < 0)
		{
			throw std::runtime_error("Error: can't add fd to epoll (epoll_ctl)");
		}	
	}
}

void HttpServer::_initialConnection(int listenFd)
{
	int clientFd = accept(listenFd, nullptr, nullptr);
	if (clientFd < 0)
		return;

	_setNonBlocking(clientFd);

	struct epoll_event clientEvent;
	clientEvent.events = EPOLLIN | EPOLLHUP;
	clientEvent.data.fd = clientFd;

	_connections[clientFd] = std::make_unique<HttpConnection>(clientFd);
	epoll_ctl(_epfd, EPOLL_CTL_ADD, clientFd, &clientEvent);
}

void HttpServer::_closeConnectionOnError(int fd)
{
	HttpResponse  errorResponse;
	
	auto connection = _connections[fd];
	_errorHandler->buildErrorResponse(InternalServerError, errorResponse);
	connection->queueResponse(errorResponse);
	close(fd);
	epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL);
}

void HttpServer::_handleInited(int fd)
{
	if (_connections.find(fd) == _connections.end())
		return;

	auto connection = _connections[fd];
	if (!connection || !_app)
		return;

	if (connection->readIntoBuffer())
	{
		auto request = connection->getRequest();
		if (request.has_value())
		{
			auto response = _app->handle(request.value());
			connection->queueResponse(response);
			close(fd);
			epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL);
		}
		else 
		{
			_closeConnectionOnError(fd);
			return;
		}
	}
	if (connection->isError())
	{
		_closeConnectionOnError(fd);
	}
}

void HttpServer::_handleEpollQue(struct epoll_event *que, int size)
{
	for (int i = 0; i < size; ++i)
	{
		int fd = que[i].data.fd;
		if (_listenFds.count(fd))
		{
			_initialConnection(fd);
		}
		else	
		{
			_handleInited(fd);
		}
	}
}

void HttpServer::setApp(std::unique_ptr<IHttpApp> app)
{
	_app = std::move(app);
}

void HttpServer::run()
{
	struct epoll_event events[MAX_EVENTS];

	while (true)
	{
		int n = epoll_wait(_epfd, events, MAX_EVENTS, -1);
		try
		{
			_handleEpollQue(events, n);
		}
		catch(const std::exception& e)
		{
			std::cout << "Error: " << e.what() << std::endl;
		}
	}	
}


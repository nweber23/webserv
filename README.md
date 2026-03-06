*This project has been created as part of the 42 curriculum by yyudi, nluchini, nweber.*

## Description

WebServ is a high-performance HTTP web server written in modern C++. It demonstrates a layered architecture with clean separation of concerns, featuring an epoll-based networking layer, HTTP parsing and formatting, and a middleware system for request handling.

The server supports static file serving, custom route handling, and connection timeout management. It is built from scratch without external web frameworks, providing educational insight into how HTTP servers work at a fundamental level.

## Features

Core networking built on epoll for scalable concurrent connection handling
Incremental HTTP request parsing that handles streaming input
Middleware-based application framework for extensible request handling
Static file serving with MIME type detection
Connection state management with activity timeouts
Clean separation between network, HTTP parsing, and application layers

## Instructions

Compilation

Run the following command to compile the project:

```
make
```

The Makefile includes build targets for the main server and test runner:
- `make` or `make all` builds the webserv executable
- `make re` performs a clean rebuild
- `make build-test` compiles tests with debug symbols
- `make test` runs the test suite
- `make clean` removes object files
- `make fclean` removes all build artifacts

Running the Server

After compilation, start the server with:

```
./webserv
```

The server listens on 0.0.0.0:8080 by default and serves files from the ./html directory.

Building with Middleware

Create a server instance using the builder pattern:

```cpp
auto app = HttpAppBuilder{}
  .useStaticFiles("./www")
  .get("/api/health", [](auto& req){ return text("ok"); })
  .build();

HttpServer server = HttpServerBuilder{}
  .listen("0.0.0.0", 8080)
  .withApp(std::move(app))
  .build();

server.run();
```

## Architecture

The project is organized into three layers:

Net Layer: Handles TCP connections, socket operations, and epoll event loop
HTTP Layer: Parses incoming HTTP requests and formats outgoing responses
App Layer: Provides middleware system and routing for business logic

Each layer has corresponding interface classes and implementations, enabling flexibility and testability.

## Resources

HTTP Semantics: https://www.rfc-editor.org/rfc/rfc9110.html
epoll Man Page: https://man7.org/linux/man-pages/man7/epoll.7.html
HTTP Cookies: https://de.wikipedia.org/wiki/HTTP-Cookie

AI Usage

AI was used for:
- Answering questions about HTTP/RFC protocol specifications
- Code reviews and architecture feedback
- Frontend design consultation
- General technical guidance on implementation questions

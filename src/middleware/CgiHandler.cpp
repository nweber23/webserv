#include "middleware/CgiHandler.hpp"

#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <cerrno>
#include <cstring>
#include <sstream>
#include <algorithm>

static std::string toUpperCopy(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return (char)std::toupper(c); });
	return s;
}

static std::string toLowerCopy(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return (char)std::tolower(c); });
	return s;
}

CgiHandler::CgiHandler(std::shared_ptr<ErrorPageHandler> errorHandler)
	: AMiddleware(errorHandler)
{}

std::string CgiHandler::trim(const std::string& s)
{
	size_t b = s.find_first_not_of(" \t\r\n");
	size_t e = s.find_last_not_of(" \t\r\n");
	if (b == std::string::npos || e == std::string::npos)
		return "";
	return s.substr(b, e - b + 1);
}

std::string CgiHandler::getHeaderValue(const HttpRequest& request, const std::string& key)
{
	auto it = request.headers.find(key);
	if (it != request.headers.end())
		return it->second;
	// Try case-insensitive lookup (simple scan)
	std::string lowKey = toLowerCopy(key);
	for (const auto& kv : request.headers)
	{
		if (toLowerCopy(kv.first) == lowKey)
			return kv.second;
	}
	return "";
}

std::string CgiHandler::resolveScriptPath(const HttpRequest& request)
{
	const auto* loc = request.location;
	std::string root = loc->root;
	std::string relPath = request.path.substr(loc->path.size());
	if (relPath.empty() || relPath[0] != '/')
		relPath = "/" + relPath;
	return root + relPath;
}

std::string CgiHandler::getFileExtension(const std::string& path)
{
	size_t dot = path.rfind('.');
	if (dot == std::string::npos)
		return "";
	return path.substr(dot);
}

std::vector<std::string> CgiHandler::buildEnv(const HttpRequest& request,
	                                           const std::string& scriptPath) const
{
	std::vector<std::string> env;

	std::string host = getHeaderValue(request, "Host");
	std::string serverName;
	std::string serverPort;
	if (!host.empty())
	{
		size_t colon = host.find(':');
		if (colon == std::string::npos)
		{
			serverName = host;
			serverPort = "80";
		}
		else
		{
			serverName = host.substr(0, colon);
			serverPort = host.substr(colon + 1);
		}
	}

	if (serverName.empty()) serverName = "localhost";
	if (serverPort.empty()) serverPort = "80";

	// Core CGI variables
	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back("SERVER_PROTOCOL=" + (request.version.empty() ? std::string("HTTP/1.1") : request.version));
	env.push_back("REQUEST_METHOD=" + request.method);
	env.push_back("QUERY_STRING=" + request.query);
	env.push_back("SCRIPT_NAME=" + request.path);
	env.push_back("SCRIPT_FILENAME=" + scriptPath);
	env.push_back("DOCUMENT_ROOT=" + request.location->root);
	env.push_back("SERVER_NAME=" + serverName);
	env.push_back("SERVER_PORT=" + serverPort);

	std::string contentType = getHeaderValue(request, "Content-Type");
	if (!contentType.empty())
		env.push_back("CONTENT_TYPE=" + contentType);

	// Body size
	env.push_back("CONTENT_LENGTH=" + std::to_string(request.body.size()));

	// Map incoming headers to HTTP_* vars
	for (const auto& kv : request.headers)
	{
		std::string k = kv.first;
		std::string v = kv.second;
		std::string low = toLowerCopy(k);
		if (low == "content-type" || low == "content-length")
			continue;
		std::string up = toUpperCopy(k);
		for (char& c : up)
			if (c == '-') c = '_';
		env.push_back("HTTP_" + up + "=" + v);
	}

	return env;
}

bool CgiHandler::runCgi(const HttpRequest& request,
	                      const std::string& interpreter,
	                      const std::string& scriptPath,
	                      std::string& cgiOutput)
{
	int inPipe[2];
	int outPipe[2];
	if (pipe(inPipe) != 0)
		return false;
	if (pipe(outPipe) != 0)
	{
		close(inPipe[0]);
		close(inPipe[1]);
		return false;
	}

	pid_t pid = fork();
	if (pid < 0)
	{
		close(inPipe[0]);
		close(inPipe[1]);
		close(outPipe[0]);
		close(outPipe[1]);
		return false;
	}

	if (pid == 0)
	{
		// Child: stdin <- inPipe[0], stdout -> outPipe[1]
		dup2(inPipe[0], STDIN_FILENO);
		dup2(outPipe[1], STDOUT_FILENO);
		// close fds
		close(inPipe[0]);
		close(inPipe[1]);
		close(outPipe[0]);
		close(outPipe[1]);

		// chdir to script directory for relative paths
		std::string dir = ".";
		size_t slash = scriptPath.find_last_of('/');
		if (slash != std::string::npos)
			dir = scriptPath.substr(0, slash);
		(void)chdir(dir.c_str());

		// argv
		const bool directExec = (interpreter == "DIRECT" || interpreter == "-" || interpreter == "");
		std::vector<std::string> argvStr;
		if (directExec)
			argvStr.push_back(scriptPath);
		else
		{
			argvStr.push_back(interpreter);
			argvStr.push_back(scriptPath);
		}
		std::vector<char*> argv;
		for (auto& s : argvStr)
			argv.push_back(const_cast<char*>(s.c_str()));
		argv.push_back(nullptr);

		// envp
		std::vector<std::string> envStr = buildEnv(request, scriptPath);
		std::vector<char*> envp;
		for (auto& s : envStr)
			envp.push_back(const_cast<char*>(s.c_str()));
		envp.push_back(nullptr);

		if (directExec)
			execve(scriptPath.c_str(), argv.data(), envp.data());
		else
			execve(interpreter.c_str(), argv.data(), envp.data());
		// If execve fails
		std::string msg = "Status: 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\nexecve failed: ";
		msg += std::strerror(errno);
		write(STDOUT_FILENO, msg.c_str(), msg.size());
		_exit(1);
	}

	// Parent
	close(inPipe[0]);
	close(outPipe[1]);

	// Write request body then close stdin pipe (signals EOF to CGI)
	const std::string& body = request.body;
	size_t written = 0;
	while (written < body.size())
	{
		ssize_t n = write(inPipe[1], body.data() + written, body.size() - written);
		if (n < 0)
			break;
		written += static_cast<size_t>(n);
	}
	close(inPipe[1]);

	// Read CGI output until EOF
	cgiOutput.clear();
	char buf[4096];
	while (true)
	{
		ssize_t n = read(outPipe[0], buf, sizeof(buf));
		if (n <= 0)
			break;
		cgiOutput.append(buf, static_cast<size_t>(n));
	}
	close(outPipe[0]);

	int status = 0;
	(void)waitpid(pid, &status, 0);
	return true;
}

bool CgiHandler::parseCgiOutput(const std::string& raw, HttpResponse& response)
{
	// Split headers/body by CRLFCRLF (fallback to LFLF)
	size_t sep = raw.find("\r\n\r\n");
	size_t sepLen = 4;
	if (sep == std::string::npos)
	{
		sep = raw.find("\n\n");
		sepLen = 2;
	}

	std::string headerBlock;
	std::string body;
	if (sep != std::string::npos)
	{
		headerBlock = raw.substr(0, sep);
		body = raw.substr(sep + sepLen);
	}
	else
	{
		// No headers: treat everything as body
		body = raw;
	}

	response.status = 200;
	response.statusText = "OK";
	response.headers.clear();
	response.body = body;

	std::istringstream hs(headerBlock);
	std::string line;
	while (std::getline(hs, line))
	{
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		if (line.empty())
			continue;
		size_t colon = line.find(':');
		if (colon == std::string::npos)
			continue;
		std::string key = trim(line.substr(0, colon));
		std::string val = trim(line.substr(colon + 1));
		std::string low = toLowerCopy(key);
		if (low == "status")
		{
			std::istringstream ss(val);
			ss >> response.status;
			std::getline(ss, response.statusText);
			response.statusText = trim(response.statusText);
			if (response.statusText.empty())
				response.statusText = "OK";
		}
		else
			response.headers[key] = val;
	}

	// Default content-type for CGI
	if (response.headers.find("Content-Type") == response.headers.end() &&
	    response.headers.find("Content-type") == response.headers.end())
		response.headers["Content-Type"] = "text/html";

	response.headers["Content-Length"] = std::to_string(response.body.size());
	return true;
}

bool CgiHandler::handle(HttpRequest& request, HttpResponse& response)
{
	if (!request.location)
		return callNext(request, response);
	if (request.location->cgi_pass.empty())
		return callNext(request, response);

	std::string scriptPath = resolveScriptPath(request);
	std::string ext = getFileExtension(scriptPath);
	if (ext.empty())
		return callNext(request, response);

	auto it = request.location->cgi_pass.find(ext);
	if (it == request.location->cgi_pass.end())
		return callNext(request, response);

	std::string interpreter = it->second;
	if (interpreter.empty())
	{
		_errorHandler->buildErrorResponse(InternalServerError, response);
		return true;
	}
	const bool directExec = (interpreter == "DIRECT" || interpreter == "-");

	struct stat st;
	if (stat(scriptPath.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
	{
		_errorHandler->buildErrorResponse(NotFound, response);
		return true;
	}
	if (directExec && access(scriptPath.c_str(), X_OK) != 0)
	{
		_errorHandler->buildErrorResponse(Forbidden, response);
		return true;
	}

	std::string cgiRaw;
	if (!runCgi(request, interpreter, scriptPath, cgiRaw))
	{
		_errorHandler->buildErrorResponse(BadGateway, response);
		return true;
	}

	if (!parseCgiOutput(cgiRaw, response))
	{
		_errorHandler->buildErrorResponse(BadGateway, response);
		return true;
	}
	return true;
}

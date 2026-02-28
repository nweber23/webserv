#include "middleware/CgiHandler.hpp"

#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <poll.h>
#include <fcntl.h>
#include <signal.h>

#include <cerrno>
#include <cstring>
#include <sstream>
#include <algorithm>

#define CGI_TIMEOUT_MS 5000

/* =========================
   Utility
   ========================= */

static long long nowMs()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long long)tv.tv_sec * 1000LL + (long long)tv.tv_usec / 1000LL;
}

static void setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags >= 0)
		fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static void killProcess(pid_t pid)
{
	if (pid > 0)
	{
		kill(pid, SIGKILL);
		waitpid(pid, NULL, 0);
	}
}

static std::string toUpperCopy(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), ::toupper);
	return s;
}

static std::string toLowerCopy(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

/* =========================
   Constructor
   ========================= */

CgiHandler::CgiHandler(std::shared_ptr<ErrorPageHandler> errorHandler)
	: AMiddleware(errorHandler)
{}

/* =========================
   Helper functions
   ========================= */

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
	std::string rel = request.path.substr(loc->path.size());

	if (rel.empty() || rel[0] != '/')
		rel = "/" + rel;

	return root + rel;
}

std::string CgiHandler::getFileExtension(const std::string& path)
{
	size_t dot = path.rfind('.');
	if (dot == std::string::npos)
		return "";

	return path.substr(dot);
}

/* =========================
   Environment builder
   ========================= */

std::vector<std::string> CgiHandler::buildEnv(
	const HttpRequest& request,
	const std::string& scriptPath) const
{
	std::vector<std::string> env;

	std::string host = getHeaderValue(request, "Host");

	std::string serverName = "localhost";
	std::string serverPort = "80";

	if (!host.empty())
	{
		size_t colon = host.find(':');

		if (colon == std::string::npos)
			serverName = host;
		else
		{
			serverName = host.substr(0, colon);
			serverPort = host.substr(colon + 1);
		}
	}

	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back("SERVER_PROTOCOL=" + request.version);
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

	env.push_back("CONTENT_LENGTH=" + std::to_string(request.body.size()));

	for (const auto& kv : request.headers)
	{
		std::string key = toUpperCopy(kv.first);

		for (char& c : key)
			if (c == '-') c = '_';

		env.push_back("HTTP_" + key + "=" + kv.second);
	}

	return env;
}

/* =========================
   Run CGI with timeout
   ========================= */

bool CgiHandler::runCgi(
	const HttpRequest& request,
	const std::string& interpreter,
	const std::string& scriptPath,
	std::string& output)
{
	int inPipe[2];
	int outPipe[2];

	if (pipe(inPipe) != 0)
		return false;

	if (pipe(outPipe) != 0)
		return false;

	pid_t pid = fork();

	if (pid < 0)
		return false;

	if (pid == 0)
	{
		dup2(inPipe[0], STDIN_FILENO);
		dup2(outPipe[1], STDOUT_FILENO);

		close(inPipe[0]);
		close(inPipe[1]);
		close(outPipe[0]);
		close(outPipe[1]);

		std::string filename = scriptPath;

		size_t slash = scriptPath.find_last_of('/');
		std::string dir = ".";

		if (slash != std::string::npos)
		{
			dir = scriptPath.substr(0, slash);
			filename = scriptPath.substr(slash + 1);
		}

		chdir(dir.c_str());

		std::vector<std::string> argvStr;

		bool directExec = (interpreter == "DIRECT" || interpreter == "-");

		if (directExec)
			argvStr.push_back(filename);
		else
		{
			argvStr.push_back(interpreter);
			argvStr.push_back(filename);
		}

		std::vector<char*> argv;

		for (auto& s : argvStr)
			argv.push_back(const_cast<char*>(s.c_str()));

		argv.push_back(NULL);

		std::vector<std::string> envStr = buildEnv(request, scriptPath);

		std::vector<char*> envp;

		for (auto& s : envStr)
			envp.push_back(const_cast<char*>(s.c_str()));

		envp.push_back(NULL);

		if (directExec)
			execve(filename.c_str(), argv.data(), envp.data());
		else
			execve(interpreter.c_str(), argv.data(), envp.data());

		exit(1);
	}

	close(inPipe[0]);
	close(outPipe[1]);

	setNonBlocking(outPipe[0]);

	write(inPipe[1], request.body.c_str(), request.body.size());
	close(inPipe[1]);

	char buf[4096];

	output.clear();

	long long deadline = nowMs() + CGI_TIMEOUT_MS;

	while (true)
	{
		ssize_t n = read(outPipe[0], buf, sizeof(buf));

		if (n > 0)
			output.append(buf, n);

		int status;

		if (waitpid(pid, &status, WNOHANG) == pid)
			break;

		if (nowMs() > deadline)
		{
			close(outPipe[0]);
			killProcess(pid);
			return false;
		}

		struct pollfd pfd;

		pfd.fd = outPipe[0];
		pfd.events = POLLIN;

		poll(&pfd, 1, 100);
	}

	close(outPipe[0]);

	return true;
}

/* =========================
   Parse CGI Output
   ========================= */

bool CgiHandler::parseCgiOutput(const std::string& raw, HttpResponse& response)
{
	size_t sep = raw.find("\r\n\r\n");

	if (sep == std::string::npos)
		sep = raw.find("\n\n");

	std::string headerBlock;
	std::string body;

	if (sep != std::string::npos)
	{
		headerBlock = raw.substr(0, sep);
		body = raw.substr(sep + 4);
	}
	else
		body = raw;

	response.status = 200;
	response.statusText = "OK";
	response.body = body;

	std::istringstream hs(headerBlock);

	std::string line;

	while (std::getline(hs, line))
	{
		if (!line.empty() && line.back() == '\r')
			line.pop_back();

		size_t colon = line.find(':');

		if (colon == std::string::npos)
			continue;

		std::string key = trim(line.substr(0, colon));
		std::string val = trim(line.substr(colon + 1));

		response.headers[key] = val;
	}

	response.headers["Content-Length"] = std::to_string(body.size());

	if (response.headers.find("Content-Type") == response.headers.end())
		response.headers["Content-Type"] = "text/html";

	return true;
}

/* =========================
   Middleware Handler
   ========================= */

bool CgiHandler::handle(HttpRequest& request, HttpResponse& response)
{
	if (!request.location)
		return callNext(request, response);

	if (request.location->cgi_pass.empty())
		return callNext(request, response);

	std::string scriptPath = resolveScriptPath(request);

	std::string ext = getFileExtension(scriptPath);

	auto it = request.location->cgi_pass.find(ext);

	if (it == request.location->cgi_pass.end())
		return callNext(request, response);

	std::string interpreter = it->second;

	struct stat st;

	if (stat(scriptPath.c_str(), &st) != 0)
	{
		_errorHandler->buildErrorResponse(NotFound, response);
		return true;
	}

	std::string raw;

	if (!runCgi(request, interpreter, scriptPath, raw))
	{
		_errorHandler->buildErrorResponse(GatewayTimeout, response);
		return true;
	}

	parseCgiOutput(raw, response);

	return true;
}
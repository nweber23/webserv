#include "CgiRunner.hpp"
#include "CgiParser.hpp"

#include <vector>
#include <sstream>
#include <cstring>
#include <cerrno>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <fcntl.h>
#include <signal.h>

static int setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) return -1;
    return 0;
}

static std::string toUpperUnderscore(const std::string &s) {
    std::string out;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '-') out += '_';
        else if (c >= 'a' && c <= 'z') out += (c - 'a' + 'A');
        else out += c;
    }
    return out;
}

static void buildEnv(const CgiRequest &req, const CgiConfig &cfg, std::vector<std::string> &envStr) {
    envStr.clear();

    // Core CGI vars
    envStr.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envStr.push_back("SERVER_PROTOCOL=" + (req.serverProtocol.empty() ? std::string("HTTP/1.1") : req.serverProtocol));
    envStr.push_back("REQUEST_METHOD=" + req.method);
    envStr.push_back("QUERY_STRING=" + req.queryString);
    envStr.push_back("SCRIPT_FILENAME=" + req.scriptFilename);
    envStr.push_back("REDIRECT_STATUS=200"); // important for php-cgi often

    if (!req.pathInfo.empty())
        envStr.push_back("PATH_INFO=" + req.pathInfo);

    // Content headers
    std::map<std::string, std::string>::const_iterator it;
    it = req.headers.find("Content-Type");
    if (it != req.headers.end())
        envStr.push_back("CONTENT_TYPE=" + it->second);

    it = req.headers.find("Content-Length");
    if (it != req.headers.end())
        envStr.push_back("CONTENT_LENGTH=" + it->second);
    else if (!req.body.empty()) {
        std::ostringstream oss;
        oss << req.body.size();
        envStr.push_back("CONTENT_LENGTH=" + oss.str());
    }

    // HTTP_* headers
    for (it = req.headers.begin(); it != req.headers.end(); ++it) {
        std::string key = it->first;
        if (key == "Content-Type" || key == "Content-Length")
            continue;
        envStr.push_back("HTTP_" + toUpperUnderscore(key) + "=" + it->second);
    }

    // Optional: some servers also set these
    // envStr.push_back("SERVER_NAME=localhost");
    // envStr.push_back("SERVER_PORT=8080");
}

static void makeEnvp(std::vector<std::string> &envStr, std::vector<char*> &envp) {
    envp.clear();
    for (size_t i = 0; i < envStr.size(); ++i)
        envp.push_back(const_cast<char*>(envStr[i].c_str()));
    envp.push_back(0);
}

static bool writeAllWithTimeout(int fd, const std::string &data, int timeoutMs, std::string &err) {
    size_t off = 0;
    while (off < data.size()) {
        struct pollfd p;
        p.fd = fd;
        p.events = POLLOUT;
        int pr = poll(&p, 1, timeoutMs);
        if (pr == 0) { err = "timeout writing to CGI stdin"; return false; }
        if (pr < 0) { err = "poll() failed while writing"; return false; }

        ssize_t w = ::write(fd, data.data() + off, data.size() - off);
        if (w < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
            err = "write() failed while sending body to CGI";
            return false;
        }
        off += static_cast<size_t>(w);
    }
    return true;
}

static bool readAllWithTimeout(int fd, int timeoutMs, std::string &out, std::string &err) {
    out.clear();
    char buf[4096];

    while (true) {
        struct pollfd p;
        p.fd = fd;
        p.events = POLLIN;
        int pr = poll(&p, 1, timeoutMs);
        if (pr == 0) { err = "timeout reading CGI stdout"; return false; }
        if (pr < 0) { err = "poll() failed while reading"; return false; }

        ssize_t r = ::read(fd, buf, sizeof(buf));
        if (r == 0) break;
        if (r < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
            err = "read() failed while reading CGI output";
            return false;
        }
        out.append(buf, buf + r);
    }
    return true;
}

static std::string parentDirFromPath(const std::string &path) {
    size_t slash = path.find_last_of('/');
    if (slash == std::string::npos) return ".";
    if (slash == 0) return "/";
    return path.substr(0, slash);
}

CgiResult runCgi(const CgiRequest &req, const CgiConfig &cfg) {
    CgiResult res;

    int inPipe[2];
    int outPipe[2];
    int errPipe[2];

    if (pipe(inPipe) < 0 || pipe(outPipe) < 0 || pipe(errPipe) < 0) {
        res.errorMsg = "pipe() failed";
        return res;
    }

    // make parent ends non-blocking (helps timeout handling)
    setNonBlocking(inPipe[1]);
    setNonBlocking(outPipe[0]);
    setNonBlocking(errPipe[0]);

    pid_t pid = fork();
    if (pid < 0) {
        res.errorMsg = "fork() failed";
        return res;
    }

    if (pid == 0) {
        // CHILD
        ::dup2(inPipe[0], STDIN_FILENO);
        ::dup2(outPipe[1], STDOUT_FILENO);
        ::dup2(errPipe[1], STDERR_FILENO);

        ::close(inPipe[0]); ::close(inPipe[1]);
        ::close(outPipe[0]); ::close(outPipe[1]);
        ::close(errPipe[0]); ::close(errPipe[1]);

        std::string wd = cfg.workingDir.empty() ? parentDirFromPath(req.scriptFilename) : cfg.workingDir;
        if (!wd.empty()) ::chdir(wd.c_str());

        std::vector<std::string> envStr;
        std::vector<char*> envp;
        buildEnv(req, cfg, envStr);
        makeEnvp(envStr, envp);

        // argv: interpreter + script file (works for python; php-cgi often uses env, but passing script is ok)
        std::vector<std::string> argvStr;
        argvStr.push_back(cfg.interpreter);
        argvStr.push_back(req.scriptFilename);

        std::vector<char*> argv;
        for (size_t i = 0; i < argvStr.size(); ++i)
            argv.push_back(const_cast<char*>(argvStr[i].c_str()));
        argv.push_back(0);

        ::execve(cfg.interpreter.c_str(), &argv[0], &envp[0]);
        // If execve fails:
        _exit(127);
    }

    // PARENT
    ::close(inPipe[0]);
    ::close(outPipe[1]);
    ::close(errPipe[1]);

    // write request body to CGI stdin, then close to signal EOF
    if (!req.body.empty()) {
        std::string werr;
        if (!writeAllWithTimeout(inPipe[1], req.body, cfg.timeoutMs, werr)) {
            res.timeout = (werr.find("timeout") != std::string::npos);
            res.errorMsg = werr;
            ::close(inPipe[1]);
            ::kill(pid, SIGKILL);
            ::waitpid(pid, 0, 0);
            return res;
        }
    }
    ::close(inPipe[1]);

    // read stdout and stderr
    std::string rerr;
    if (!readAllWithTimeout(outPipe[0], cfg.timeoutMs, res.rawStdout, rerr)) {
        res.timeout = (rerr.find("timeout") != std::string::npos);
        res.errorMsg = rerr;
        ::close(outPipe[0]);
        ::close(errPipe[0]);
        ::kill(pid, SIGKILL);
        ::waitpid(pid, 0, 0);
        return res;
    }
    ::close(outPipe[0]);

    // best-effort read stderr (no hard fail if empty)
    std::string eerr;
    readAllWithTimeout(errPipe[0], cfg.timeoutMs, res.rawStderr, eerr);
    ::close(errPipe[0]);

    int status = 0;
    ::waitpid(pid, &status, 0);
    res.exitStatus = status;

    // parse CGI output into HTTP pieces
    ParsedCgiOutput parsed;
    if (!parseCgiOutput(res.rawStdout, parsed)) {
        res.errorMsg = "failed to parse CGI output";
        res.httpStatus = 500;
        res.ok = false;
        return res;
    }

    res.httpStatus = parsed.status;
    res.outHeaders = parsed.headers;
    res.outBody = parsed.body;

    // minimal sanity: must have Content-Type or we set a default
    if (res.outHeaders.find("Content-Type") == res.outHeaders.end())
        res.outHeaders["Content-Type"] = "text/plain";

    res.ok = true;
    return res;
}

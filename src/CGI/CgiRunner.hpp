/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRunner.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyudi <yyudi@student.42heilbronn.de>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 15:33:57 by yyudi             #+#    #+#             */
/*   Updated: 2026/01/12 15:33:58 by yyudi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_RUNNER_HPP
#define CGI_RUNNER_HPP

#include <string>
#include <map>

struct CgiRequest {
    std::string method;        // "GET", "POST", ...
    std::string scriptFilename; // full path to script file on disk
    std::string queryString;   // without '?', can be empty
    std::string pathInfo;      // optional
    std::string serverProtocol; // "HTTP/1.1"
    std::map<std::string, std::string> headers;
    std::string body;          // already collected full body (unchunked if needed)
};

struct CgiConfig {
    std::string interpreter;   // e.g. "/usr/bin/python3" or "/usr/bin/php-cgi"
    std::string workingDir;    // directory to chdir() before exec (can be script dir)
    int timeoutMs;             // e.g. 3000
};

struct CgiResult {
    bool ok;
    bool timeout;
    int exitStatus;            // waitpid status (raw)
    int httpStatus;            // parsed from CGI output "Status:" else 200/500
    std::map<std::string, std::string> outHeaders;
    std::string outBody;

    // For debugging / logging
    std::string rawStdout;
    std::string rawStderr;
    std::string errorMsg;

    CgiResult() : ok(false), timeout(false), exitStatus(0), httpStatus(500) {}
};

CgiResult runCgi(const CgiRequest &req, const CgiConfig &cfg);

#endif

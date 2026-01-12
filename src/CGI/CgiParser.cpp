/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiParser.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyudi <yyudi@student.42heilbronn.de>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 15:34:16 by yyudi             #+#    #+#             */
/*   Updated: 2026/01/12 15:34:17 by yyudi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiParser.hpp"
#include <sstream>
#include <cctype>

static std::string trim(const std::string &s) {
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) i++;
    size_t j = s.size();
    while (j > i && std::isspace(static_cast<unsigned char>(s[j - 1]))) j--;
    return s.substr(i, j - i);
}

static bool splitHeadersBody(const std::string &raw, std::string &h, std::string &b) {
    size_t pos = raw.find("\r\n\r\n");
    if (pos != std::string::npos) {
        h = raw.substr(0, pos);
        b = raw.substr(pos + 4);
        return true;
    }
    pos = raw.find("\n\n");
    if (pos != std::string::npos) {
        h = raw.substr(0, pos);
        b = raw.substr(pos + 2);
        return true;
    }
    // no header separator found; treat all as body
    h.clear();
    b = raw;
    return true;
}

bool parseCgiOutput(const std::string &raw, ParsedCgiOutput &out) {
    std::string headerPart, bodyPart;
    if (!splitHeadersBody(raw, headerPart, bodyPart))
        return false;

    out.body = bodyPart;
    out.status = 200;
    out.headers.clear();

    std::istringstream iss(headerPart);
    std::string line;
    while (std::getline(iss, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        if (line.empty())
            continue;

        size_t colon = line.find(':');
        if (colon == std::string::npos)
            continue;

        std::string key = trim(line.substr(0, colon));
        std::string val = trim(line.substr(colon + 1));

        if (key == "Status") {
            // format: "Status: 302 Found"
            std::istringstream ss(val);
            int code = 0;
            ss >> code;
            if (code >= 100 && code <= 599)
                out.status = code;
        } else {
            out.headers[key] = val;
        }
    }
    return true;
}

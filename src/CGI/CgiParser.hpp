/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiParser.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyudi <yyudi@student.42heilbronn.de>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 15:34:07 by yyudi             #+#    #+#             */
/*   Updated: 2026/01/12 15:34:10 by yyudi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_PARSER_HPP
#define CGI_PARSER_HPP

#include <string>
#include <map>

struct ParsedCgiOutput {
    int status; // default 200
    std::map<std::string, std::string> headers;
    std::string body;

    ParsedCgiOutput() : status(200) {}
};

bool parseCgiOutput(const std::string &raw, ParsedCgiOutput &out);

#endif

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaoh <jaoh@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/20 16:24:57 by jaoh              #+#    #+#             */
/*   Updated: 2026/01/29 16:53:21 by jaoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>

class HTTPRequest {
public:
    HTTPRequest();

    // raw data 누적 (recv 결과)
    bool appendData(const std::string& data);

    bool isComplete() const;
    bool hasError() const;

    const std::string& getMethod() const;
    const std::string& getURI() const;
    const std::string& getVersion() const;
    const std::map<std::string, std::string>& getHeaders() const;
    const std::string& getBody() const;

private:
    void parseRequestLine(const std::string& line);
    void parseHeaders(const std::string& block);
    bool parseBody();
    bool parseChunkedBody();

    std::string trim(const std::string& s) const;
    std::string toLower(const std::string& s) const;

private:
    std::string rawBuffer;

    std::string method;
    std::string uri;
    std::string version;

    std::map<std::string, std::string> headers;
    std::string body;

    bool headersParsed;
    bool bodyParsed;
    bool complete;
    bool error;

    size_t contentLength;
    bool chunked;
    size_t bodyStart;
};

#endif

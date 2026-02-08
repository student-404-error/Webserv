/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaoh <jaoh@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/12/20 16:27:49 by jaoh              #+#    #+#             */
/*   Updated: 2026/02/08 14:53:31 by jaoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include <sstream>

// 상수 정의
namespace {
    const int DEFAULT_STATUS_CODE = 200;
    const std::string DEFAULT_STATUS_MESSAGE = "OK";
    const std::string DEFAULT_CONTENT_TYPE = "text/html";
    const std::string DEFAULT_CONNECTION = "close";
    const std::string Http_VERSION = "HTTP/1.1";
    const std::string CRLF = "\r\n";
    const std::string HEADER_SEPARATOR = ": ";
}

HttpResponse::HttpResponse() 
    : statusCode(DEFAULT_STATUS_CODE), 
      statusMessage(DEFAULT_STATUS_MESSAGE) {}

void HttpResponse::setStatus(int code) {
    statusCode = code;
    statusMessage = getStatusMessage(code);
}

void HttpResponse::setHeader(const std::string& key, const std::string& value) {
    headers[key] = value;
}

void HttpResponse::setContentType(const std::string& type) {
    headers["Content-Type"] = type;
}

void HttpResponse::setBody(const std::string& body) {
    this->body = body;
    
    // Content-Length 자동 계산
    std::ostringstream oss;
    oss << body.size();
    headers["Content-Length"] = oss.str();
}

std::string HttpResponse::getStatusMessage(int code) const {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 413: return "Payload Too Large";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        default:  return "Unknown";
    }
}

void HttpResponse::ensureHeaders() {
    // Content-Length가 없으면 본문 크기로 자동 설정
    if (headers.find("Content-Length") == headers.end()) {
        std::ostringstream oss;
        oss << body.size();
        headers["Content-Length"] = oss.str();
    }
    
    // Content-Type이 없으면 기본값 설정
    if (headers.find("Content-Type") == headers.end()) {
        headers["Content-Type"] = DEFAULT_CONTENT_TYPE;
    }
    
    // Connection 헤더가 없으면 기본값 설정
    if (headers.find("Connection") == headers.end()) {
        headers["Connection"] = DEFAULT_CONNECTION;
    }
}

std::string HttpResponse::toString() {
    ensureHeaders();

    std::ostringstream oss;
    
    // 상태 라인: HTTP/1.1 200 OK
    oss << Http_VERSION << " " << statusCode << " " << statusMessage << CRLF;

    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); 
         it != headers.end(); ++it) {
        oss << it->first << HEADER_SEPARATOR << it->second << CRLF;
    }

    // 헤더와 본문 구분 (빈 줄)
    oss << CRLF;
    
    // 본문
    oss << body;
    
    return oss.str();
}

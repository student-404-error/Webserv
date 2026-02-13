/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaoh <jaoh@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 01:03:01 by jaoh              #+#    #+#             */
/*   Updated: 2026/02/12 01:03:07 by jaoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ErrorHandler.hpp"
#include <fstream>
#include <sstream>
#include <sys/stat.h>

/* ================= Public Methods ================= */

HttpResponse ErrorHandler::buildError(int code) {
    std::map<int, std::string> empty;
    return buildError(code, empty);
}

HttpResponse ErrorHandler::buildError(int code,
                                     const std::map<int, std::string>& errorPages) {
    HttpResponse response;
    response.setStatus(code);
    response.setContentType("text/html");

    // 커스텀 에러 페이지 확인
    std::map<int, std::string>::const_iterator it = errorPages.find(code);
    if (it != errorPages.end()) {
        std::string customPage = readErrorPage(it->second);
        if (!customPage.empty()) {
            response.setBody(customPage);
            return response;
        }
    }

    // 기본 에러 페이지
    std::string message = getErrorMessage(code);
    std::string body = getDefaultErrorPage(code, message);
    response.setBody(body);
    
    return response;
}

/* ================= Private Methods ================= */

std::string ErrorHandler::getDefaultErrorPage(int code, const std::string& message) {
    std::ostringstream html;
    
    html << "<!DOCTYPE html>\n";
    html << "<html>\n";
    html << "<head>\n";
    html << "<meta charset=\"UTF-8\">\n";
    html << "<title>" << code << " " << message << "</title>\n";
    html << "<style>\n";
    html << "body {\n";
    html << "  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;\n";
    html << "  display: flex;\n";
    html << "  align-items: center;\n";
    html << "  justify-content: center;\n";
    html << "  height: 100vh;\n";
    html << "  margin: 0;\n";
    html << "  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);\n";
    html << "  color: white;\n";
    html << "}\n";
    html << ".container {\n";
    html << "  text-align: center;\n";
    html << "  padding: 40px;\n";
    html << "  background: rgba(255, 255, 255, 0.1);\n";
    html << "  border-radius: 20px;\n";
    html << "  backdrop-filter: blur(10px);\n";
    html << "  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);\n";
    html << "}\n";
    html << "h1 {\n";
    html << "  font-size: 72px;\n";
    html << "  margin: 0;\n";
    html << "  font-weight: 700;\n";
    html << "}\n";
    html << "p {\n";
    html << "  font-size: 24px;\n";
    html << "  margin: 20px 0 0 0;\n";
    html << "  opacity: 0.9;\n";
    html << "}\n";
    html << ".code {\n";
    html << "  font-size: 120px;\n";
    html << "  font-weight: 900;\n";
    html << "  margin: 0;\n";
    html << "  line-height: 1;\n";
    html << "  text-shadow: 0 4px 6px rgba(0, 0, 0, 0.3);\n";
    html << "}\n";
    html << "</style>\n";
    html << "</head>\n";
    html << "<body>\n";
    html << "<div class=\"container\">\n";
    html << "<div class=\"code\">" << code << "</div>\n";
    html << "<h1>" << message << "</h1>\n";
    html << "<p>webserv/1.0</p>\n";
    html << "</div>\n";
    html << "</body>\n";
    html << "</html>\n";
    
    return html.str();
}

std::string ErrorHandler::readErrorPage(const std::string& path) {
    // 파일 존재 확인
    struct stat st;
    if (stat(path.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
        return "";

    // 파일 읽기
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open())
        return "";

    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

std::string ErrorHandler::getErrorMessage(int code) {
    switch (code) {
        // 2xx Success
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        
        // 3xx Redirection
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 304: return "Not Modified";
        case 307: return "Temporary Redirect";
        case 308: return "Permanent Redirect";
        
        // 4xx Client Error
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 408: return "Request Timeout";
        case 409: return "Conflict";
        case 410: return "Gone";
        case 411: return "Length Required";
        case 413: return "Payload Too Large";
        case 414: return "URI Too Long";
        case 415: return "Unsupported Media Type";
        case 429: return "Too Many Requests";
        
        // 5xx Server Error
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        case 504: return "Gateway Timeout";
        case 505: return "HTTP Version Not Supported";
        
        default:  return "Unknown Error";
    }
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaoh <jaoh@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/20 16:24:43 by jaoh              #+#    #+#             */
/*   Updated: 2026/02/12 01:19:17 by jaoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include <sstream>
#include <cstdlib>
#include <cctype>

// 기본 생성자: 각 플래그와 상태값을 초기화한다.
HttpRequest::HttpRequest()
    : headersParsed(false),
      bodyParsed(false),
      complete(false),
      error(false),
      contentLength(0),
      chunked(false),
      bodyStart(0) {}

// 소켓에서 새로 읽은 데이터를 rawBuffer에 이어 붙이고,
// 헤더/바디를 순차적으로 파싱한다.
// complete가 true가 되면 하나의 HTTP 요청이 완성된 것.
bool HttpRequest::appendData(const std::string& data) {
    rawBuffer += data;

    // 아직 헤더를 다 읽지 못했다면, 헤더 끝("\r\n\r\n")을 찾는다.
    if (!headersParsed) {
        size_t pos = rawBuffer.find("\r\n\r\n");
        if (pos == std::string::npos)
            return false;

        // 헤더 블록(첫 줄 + 헤더들)을 잘라서 파싱
        parseHeaders(rawBuffer.substr(0, pos));
        bodyStart = pos + 4;
        headersParsed = true;
    }

    // 헤더 파싱이 끝난 이후에는 바디를 파싱
    if (!bodyParsed) {
        if (!parseBody())
            return false;
    }

    return complete;
}

/* ================= Request Line + Headers ================= */

// "GET /index.html HTTP/1.1" 형식의 첫 줄을 method, uri, version 으로 분리
void HttpRequest::parseRequestLine(const std::string& line) {
    std::stringstream ss(line);
    ss >> method >> uri >> version;

    if (method.empty() || uri.empty() || version.empty())
        error = true;
}

// 첫 줄(요청 라인) 이후의 헤더들을 한 줄씩 읽어 파싱
void HttpRequest::parseHeaders(const std::string& block) {
    std::istringstream iss(block);
    std::string line;

    // 첫 줄은 요청 라인
    std::getline(iss, line);
    if (line.size() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1);
    parseRequestLine(line);

    // 나머지 줄들은 "Key: Value" 형식의 헤더
    while (std::getline(iss, line)) {
        if (line.size() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        if (line.empty())
            break;

        size_t pos = line.find(":");
        if (pos == std::string::npos) {
            error = true;
            return;
        }

        std::string key = toLower(trim(line.substr(0, pos)));
        std::string value = trim(line.substr(pos + 1));

        headers[key] = value;
    }

    // Content-Length 가 있으면 바디 길이 설정
    if (headers.count("content-length")) {
        int len = std::atoi(headers["content-length"].c_str());
        contentLength = (len > 0) ? static_cast<size_t>(len) : 0;
    }

    // Transfer-Encoding: chunked 인 경우 chunked 플래그 설정
    if (headers.count("transfer-encoding")
        && toLower(headers["transfer-encoding"]) == "chunked")
        chunked = true;
}

/* ================= Body ================= */

// Content-Length 또는 chunked 여부에 따라 body를 파싱
bool HttpRequest::parseBody() {
    if (chunked)
        return parseChunkedBody();

    if (contentLength > 0) {
        if (rawBuffer.size() < bodyStart + contentLength)
            return false;

        body = rawBuffer.substr(bodyStart, contentLength);
    }

    bodyParsed = true;
    complete = true;
    return true;
}

/* ================= Chunked ================= */

bool HttpRequest::parseChunkedBody() {
    size_t pos = bodyStart;

    while (true) {
        size_t crlf = rawBuffer.find("\r\n", pos);
        if (crlf == std::string::npos)
            return false;

        std::string hexSize = rawBuffer.substr(pos, crlf - pos);
        size_t chunkSize = std::strtoul(hexSize.c_str(), NULL, 16);

        pos = crlf + 2;

        if (chunkSize == 0) {
            bodyParsed = true;
            complete = true;
            return true;
        }

        if (rawBuffer.size() < pos + chunkSize + 2)
            return false;

        body.append(rawBuffer.substr(pos, chunkSize));
        pos += chunkSize + 2; // skip data + CRLF
    }
}

/* ================= Utils ================= */

std::string HttpRequest::trim(const std::string& s) const {
    size_t start = 0;
    while (start < s.size() && std::isspace(s[start]))
        start++;
    size_t end = s.size();
    while (end > start && std::isspace(s[end - 1]))
        end--;
    return s.substr(start, end - start);
}

std::string HttpRequest::toLower(const std::string& s) const {
    std::string r;
    for (size_t i = 0; i < s.size(); i++)
        r += std::tolower(s[i]);
    return r;
}

/* ================= Getters ================= */

bool HttpRequest::isComplete() const { return complete; }
bool HttpRequest::hasError() const { return error; }

const std::string& HttpRequest::getMethod() const { return method; }
const std::string& HttpRequest::getURI() const { return uri; }
const std::string& HttpRequest::getVersion() const { return version; }
const std::map<std::string, std::string>& HttpRequest::getHeaders() const { return headers; }
const std::string& HttpRequest::getBody() const { return body; }

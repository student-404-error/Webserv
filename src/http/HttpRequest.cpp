/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaoh <jaoh@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/20 16:24:43 by jaoh              #+#    #+#             */
/*   Updated: 2026/01/29 16:53:45 by jaoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPRequest.hpp"
#include <sstream>
#include <cstdlib>
#include <cctype>

// 상수 정의
namespace {
    const std::string HEADER_END_MARKER = "\r\n\r\n";  // 헤더와 본문 구분자
    const std::string CRLF = "\r\n";                    // 줄바꿈
    const std::string HEADER_SEPARATOR = ":";           // 헤더 키-값 구분자
    const std::string CHUNKED_ENCODING = "chunked";     // Chunked 전송 인코딩
    const std::string CONTENT_LENGTH_HEADER = "content-length";
    const std::string TRANSFER_ENCODING_HEADER = "transfer-encoding";
    const size_t HEADER_END_SIZE = 4;                   // "\r\n\r\n" 크기
    
    // 헬퍼 함수: 줄 끝의 \r 제거
    void removeTrailingCR(std::string& line) {
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }
    }
}

HTTPRequest::HTTPRequest()
    : headersParsed(false),
      bodyParsed(false),
      complete(false),
      error(false),
      contentLength(0),
      chunked(false),
      bodyStart(0) {}

bool HTTPRequest::appendData(const std::string& data) {
    rawBuffer += data;

    // 1단계: 헤더 파싱 (아직 완료되지 않은 경우)
    if (!headersParsed) {
        size_t headerEndPos = rawBuffer.find(HEADER_END_MARKER);
        if (headerEndPos == std::string::npos) {
            // 헤더 끝을 찾지 못함 - 더 많은 데이터 필요
            return false;
        }

        // 헤더 블록 파싱
        parseHeaders(rawBuffer.substr(0, headerEndPos));
        bodyStart = headerEndPos + HEADER_END_SIZE;
        headersParsed = true;
    }

    // 2단계: 본문 파싱 (헤더 파싱 완료 후)
    if (!bodyParsed) {
        if (!parseBody()) {
            // 본문이 아직 완전히 수신되지 않음
            return false;
        }
    }

    return complete;
}

/* ================= Request Line + Headers ================= */

void HTTPRequest::parseRequestLine(const std::string& line) {
    std::stringstream ss(line);
    ss >> method >> uri >> version;

    // 요청 라인 형식 검증: 세 요소가 모두 있어야 함
    if (method.empty() || uri.empty() || version.empty()) {
        error = true;
    }
}

void HTTPRequest::parseHeaders(const std::string& block) {
    std::istringstream iss(block);
    std::string line;

    // 첫 번째 줄: 요청 라인 파싱
    std::getline(iss, line);
    removeTrailingCR(line);
    parseRequestLine(line);

    // 나머지 줄들: 헤더 파싱
    while (std::getline(iss, line)) {
        removeTrailingCR(line);
        
        // 빈 줄이면 헤더 블록 종료
        if (line.empty()) {
            break;
        }

        // 헤더 형식: "Key: Value"
        size_t colonPos = line.find(HEADER_SEPARATOR);
        if (colonPos == std::string::npos) {
            error = true;
            return;
        }

        std::string key = toLower(trim(line.substr(0, colonPos)));
        std::string value = trim(line.substr(colonPos + 1));

        headers[key] = value;
    }

    // Content-Length 헤더 처리
    if (headers.count(CONTENT_LENGTH_HEADER)) {
        int len = std::atoi(headers[CONTENT_LENGTH_HEADER].c_str());
        contentLength = (len > 0) ? static_cast<size_t>(len) : 0;
    }

    // Transfer-Encoding: chunked 확인
    if (headers.count(TRANSFER_ENCODING_HEADER)
        && toLower(headers[TRANSFER_ENCODING_HEADER]) == CHUNKED_ENCODING) {
        chunked = true;
    }
}

/* ================= Body ================= */

bool HTTPRequest::parseBody() {
    // Chunked 전송 인코딩인 경우
    if (chunked) {
        return parseChunkedBody();
    }

    // Content-Length 기반 본문 파싱
    if (contentLength > 0) {
        // 필요한 만큼의 데이터가 아직 수신되지 않음
        if (rawBuffer.size() < bodyStart + contentLength) {
            return false;
        }

        // 본문 추출
        body = rawBuffer.substr(bodyStart, contentLength);
    }

    bodyParsed = true;
    complete = true;
    return true;
}

/* ================= Chunked ================= */

bool HTTPRequest::parseChunkedBody() {
    size_t currentPos = bodyStart;

    while (true) {
        // 청크 크기 라인 찾기 (16진수)
        size_t crlfPos = rawBuffer.find(CRLF, currentPos);
        if (crlfPos == std::string::npos) {
            // 청크 크기 라인을 찾지 못함 - 더 많은 데이터 필요
            return false;
        }

        // 청크 크기 파싱 (16진수 문자열)
        std::string hexSizeStr = rawBuffer.substr(currentPos, crlfPos - currentPos);
        size_t chunkSize = std::strtoul(hexSizeStr.c_str(), NULL, 16);

        // 데이터 시작 위치로 이동 (크기 라인 + CRLF 건너뛰기)
        currentPos = crlfPos + CRLF.size();

        // 크기가 0이면 마지막 청크 (본문 종료)
        if (chunkSize == 0) {
            bodyParsed = true;
            complete = true;
            return true;
        }

        // 청크 데이터가 완전히 수신되었는지 확인
        if (rawBuffer.size() < currentPos + chunkSize + CRLF.size()) {
            return false;
        }

        // 청크 데이터를 본문에 추가
        body.append(rawBuffer.substr(currentPos, chunkSize));
        
        // 다음 청크로 이동 (데이터 + CRLF 건너뛰기)
        currentPos += chunkSize + CRLF.size();
    }
}

/* ================= Utils ================= */

std::string HTTPRequest::trim(const std::string& s) const {
    // 앞쪽 공백 제거
    size_t start = 0;
    while (start < s.size() && std::isspace(s[start])) {
        start++;
    }
    
    // 뒤쪽 공백 제거
    size_t end = s.size();
    while (end > start && std::isspace(s[end - 1])) {
        end--;
    }
    
    return s.substr(start, end - start);
}

std::string HTTPRequest::toLower(const std::string& s) const {
    std::string result;
    result.reserve(s.size());  // 메모리 할당 최적화
    
    for (size_t i = 0; i < s.size(); i++) {
        result += std::tolower(s[i]);
    }
    
    return result;
}

/* ================= Getters ================= */

bool HTTPRequest::isComplete() const {
    return complete;
}

bool HTTPRequest::hasError() const {
    return error;
}

const std::string& HTTPRequest::getMethod() const {
    return method;
}

const std::string& HTTPRequest::getURI() const {
    return uri;
}

const std::string& HTTPRequest::getVersion() const {
    return version;
}

const std::map<std::string, std::string>& HTTPRequest::getHeaders() const {
    return headers;
}

const std::string& HTTPRequest::getBody() const {
    return body;
}

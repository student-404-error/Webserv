/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaoh <jaoh@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 17:53:03 by jaoh              #+#    #+#             */
/*   Updated: 2026/02/08 15:40:47 by jaoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PostHandler.hpp"
#include <sstream>
#include <cstdlib>
#include <cctype>

POSTHandler::POSTHandler(size_t maxBodySizeBytes) : maxBodySize(maxBodySizeBytes) {}

HttpResponse POSTHandler::handle(const HttpRequest& request,
                                 const LocationConfig& location) {
    (void)location;
    HttpResponse res;

    // body size limit 체크 (413)
    if (!checkBodySize(request, res))
        return res;

    // Content-Type 분기
    std::string ct = "";
    const std::map<std::string,std::string>& h = request.getHeaders();
    std::map<std::string,std::string>::const_iterator it = h.find("content-type");
    if (it != h.end()) ct = it->second;

    std::string lower = toLower(ct);

    // multipart/form-data; boundary=----WebKitFormBoundary...
    if (lower.find("multipart/form-data") != std::string::npos) {
        size_t bpos = lower.find("boundary=");
        if (bpos == std::string::npos) {
            res.setStatus(400);
            res.setBody("<h1>400 Bad Request</h1>");
            return res;
        }
        std::string boundary = ct.substr(bpos + 9); // 원문에서 boundary= 뒤
        boundary = trim(boundary);
        // boundary 값이 "..." 형태로 올 수도 있음
        if (boundary.size() >= 2 && boundary[0] == '"' && boundary[boundary.size()-1] == '"')
            boundary = boundary.substr(1, boundary.size()-2);

        return handleMultipart(request, location, boundary);
    }

    // application/x-www-form-urlencoded
    if (lower.find("application/x-www-form-urlencoded") != std::string::npos) {
        return handleFormUrlEncoded(request, location);
    }

    // 그 외: raw body (예: application/json 등) - 프로젝트 요구에 따라 처리
    // 여기선 일단 200 OK로 echo 예시
    res.setStatus(200);
    res.setContentType("text/plain");
    res.setBody(request.getBody());
    return res;
}

bool POSTHandler::checkBodySize(const HttpRequest& request, HttpResponse& res) const {
    if (maxBodySize == 0) return true; // 0이면 제한 없음으로 팀 규칙 가능
    if (request.getBody().size() > maxBodySize) {
        res.setStatus(413);
        res.setBody("<h1>413 Payload Too Large</h1>");
        return false;
    }
    return true;
}

/* ---------------- urlencoded ---------------- */

HttpResponse POSTHandler::handleFormUrlEncoded(const HttpRequest& request,
                                               const LocationConfig& location) {
    (void)location;
    HttpResponse res;

    std::map<std::string,std::string> kv = parseUrlEncoded(request.getBody());

    // 예시: 파싱 결과를 text로 반환
    std::ostringstream oss;
    oss << "Parsed x-www-form-urlencoded:\n";
    for (std::map<std::string,std::string>::iterator it = kv.begin(); it != kv.end(); ++it)
        oss << it->first << " = " << it->second << "\n";

    res.setStatus(200);
    res.setContentType("text/plain");
    res.setBody(oss.str());
    return res;
}

std::map<std::string,std::string> POSTHandler::parseUrlEncoded(const std::string& body) const {
    std::map<std::string,std::string> out;
    size_t i = 0;
    while (i < body.size()) {
        size_t amp = body.find('&', i);
        std::string pair = (amp == std::string::npos) ? body.substr(i) : body.substr(i, amp - i);

        size_t eq = pair.find('=');
        std::string k = (eq == std::string::npos) ? pair : pair.substr(0, eq);
        std::string v = (eq == std::string::npos) ? "" : pair.substr(eq + 1);

        out[urlDecode(k)] = urlDecode(v);

        if (amp == std::string::npos) break;
        i = amp + 1;
    }
    return out;
}

std::string POSTHandler::urlDecode(const std::string& s) const {
    std::string out;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '+') out += ' ';
        else if (s[i] == '%' && i + 2 < s.size()) {
            char hex[3] = { s[i+1], s[i+2], 0 };
            out += static_cast<char>(std::strtoul(hex, 0, 16));
            i += 2;
        } else out += s[i];
    }
    return out;
}

/* ---------------- multipart ---------------- */

HttpResponse POSTHandler::handleMultipart(const HttpRequest& request,
                                          const LocationConfig& location,
                                          const std::string& boundary) {
    (void)location;
    HttpResponse res;

    std::vector< std::map<std::string,std::string> > parts;
    if (!parseMultipart(request.getBody(), boundary, parts)) {
        res.setStatus(400);
        res.setBody("<h1>400 Bad Request</h1>");
        return res;
    }

    // TODO (Person C 연동):
    // - parts를 순회하며 file part면 location.uploadDir에 저장
    // - field part면 폼 데이터로 처리
    // - 저장 성공 시 201 Created 또는 200 OK

    std::ostringstream oss;
    oss << "Parsed multipart parts: " << parts.size() << "\n";
    for (size_t i = 0; i < parts.size(); ++i) {
        oss << "---- part " << i << "\n";
        for (std::map<std::string,std::string>::iterator it = parts[i].begin(); it != parts[i].end(); ++it) {
            oss << it->first << ": " << it->second << "\n";
        }
    }

    res.setStatus(200);
    res.setContentType("text/plain");
    res.setBody(oss.str());
    return res;
}

/*
** parts[i] map keys 예시:
** - disposition
** - name
** - filename (있을 때)
** - content-type (있을 때)
** - data (원본 바이트; 파일이면 그대로)
*/
bool POSTHandler::parseMultipart(const std::string& body,
                                 const std::string& boundary,
                                 std::vector< std::map<std::string,std::string> >& parts) const {
    std::string delim = "--" + boundary;
    std::string close = "--" + boundary + "--";

    size_t pos = 0;

    // 첫 boundary 찾기
    size_t first = body.find(delim, pos);
    if (first == std::string::npos) return false;
    pos = first;

    while (true) {
        // boundary line
        if (body.compare(pos, close.size(), close) == 0)
            return true;

        if (body.compare(pos, delim.size(), delim) != 0)
            return false;

        pos += delim.size();

        // boundary line 뒤는 \r\n 이어야 정상
        if (pos + 2 > body.size() || body.substr(pos, 2) != "\r\n")
            return false;
        pos += 2;

        // headers 끝 \r\n\r\n
        size_t hdrEnd = body.find("\r\n\r\n", pos);
        if (hdrEnd == std::string::npos) return false;

        std::string hdrBlock = body.substr(pos, hdrEnd - pos);
        pos = hdrEnd + 4;

        // 다음 boundary 위치 (data 끝)
        size_t next = body.find("\r\n" + delim, pos);
        if (next == std::string::npos) {
            // 마지막 close로 끝날 수도 있으므로 close도 확인
            next = body.find("\r\n" + close, pos);
            if (next == std::string::npos) return false;
        }

        std::string data = body.substr(pos, next - pos);
        pos = next + 2; // skip leading \r\n before boundary

        // header 파싱 (간단 버전)
        std::map<std::string,std::string> part;
        part["data"] = data;

        std::istringstream iss(hdrBlock);
        std::string line;
        while (std::getline(iss, line)) {
            if (!line.empty() && line[line.size()-1] == '\r')
                line.erase(line.size()-1);
            size_t c = line.find(':');
            if (c == std::string::npos) continue;

            std::string k = toLower(trim(line.substr(0,c)));
            std::string v = trim(line.substr(c+1));
            part[k] = v;

            // Content-Disposition에서 name/filename 뽑기 (간단)
            if (k == "content-disposition") {
                part["disposition"] = v;
                size_t npos = v.find("name=");
                if (npos != std::string::npos) {
                    size_t start = v.find('"', npos);
                    size_t end = (start!=std::string::npos)? v.find('"', start+1) : std::string::npos;
                    if (start!=std::string::npos && end!=std::string::npos)
                        part["name"] = v.substr(start+1, end-start-1);
                }
                size_t fpos = v.find("filename=");
                if (fpos != std::string::npos) {
                    size_t start = v.find('"', fpos);
                    size_t end = (start!=std::string::npos)? v.find('"', start+1) : std::string::npos;
                    if (start!=std::string::npos && end!=std::string::npos)
                        part["filename"] = v.substr(start+1, end-start-1);
                }
            }
        }

        parts.push_back(part);

        // 다음 루프는 pos가 boundary 시작을 가리킴 (delim 또는 close)
        if (body.compare(pos, close.size(), close) == 0)
            return true;
    }
}

/* ---------------- small utils ---------------- */

std::string POSTHandler::trim(const std::string& s) const {
    size_t a = 0;
    while (a < s.size() && std::isspace(static_cast<unsigned char>(s[a]))) a++;
    size_t b = s.size();
    while (b > a && std::isspace(static_cast<unsigned char>(s[b-1]))) b--;
    return s.substr(a, b-a);
}

std::string POSTHandler::toLower(const std::string& s) const {
    std::string r;
    for (size_t i=0;i<s.size();++i) r += static_cast<char>(std::tolower(static_cast<unsigned char>(s[i])));
    return r;
}

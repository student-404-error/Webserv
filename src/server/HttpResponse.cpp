#include "HttpResponse.hpp"
#include <sstream>

HttpResponse::HttpResponse() : _code(200), _reason("OK") {}

void HttpResponse::setStatus(int code, const std::string& reason) {
    _code = code;
    _reason = reason;
}

void HttpResponse::setBody(const std::string& body) { _body = body; }

void HttpResponse::addHeader(const std::string& key, const std::string& value) {
    _headers[key] = value;
}

void HttpResponse::addSetCookie(const std::string& cookieLine) {
    _setCookies.push_back(cookieLine);
}

std::string HttpResponse::itoa(int n) {
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

std::string HttpResponse::toBytes(bool keepAlive) const {
    std::ostringstream out;
    out << "HTTP/1.1 " << _code << " " << _reason << "\r\n";

    // Copy headers, but ensure required ones exist
    std::map<std::string, std::string> hdr = _headers;

    if (hdr.find("Content-Length") == hdr.end())
        hdr["Content-Length"] = itoa(static_cast<int>(_body.size()));
    if (hdr.find("Content-Type") == hdr.end())
        hdr["Content-Type"] = "text/plain; charset=utf-8";

    hdr["Connection"] = keepAlive ? "keep-alive" : "close";

    for (std::map<std::string, std::string>::const_iterator it = hdr.begin(); it != hdr.end(); ++it)
        out << it->first << ": " << it->second << "\r\n";

    for (size_t i = 0; i < _setCookies.size(); ++i)
        out << "Set-Cookie: " << _setCookies[i] << "\r\n";

    out << "\r\n";
    out << _body;

    return out.str();
}

#include "HttpRequest.hpp"
#include <sstream>
#include <cctype>
#include <cstdlib>

HttpRequest::HttpRequest() { reset(); }

void HttpRequest::reset() {
    _method = UNKNOWN;
    _target.clear();
    _path.clear();
    _query.clear();
    _version.clear();
    _headers.clear();
    _cookies.clear();
    _body.clear();
    _complete = false;
    _badRequest = false;
}

bool HttpRequest::isComplete() const { return _complete; }
bool HttpRequest::badRequest() const { return _badRequest; }

HttpRequest::Method HttpRequest::method() const { return _method; }
const std::string& HttpRequest::target() const { return _target; }
const std::string& HttpRequest::path() const { return _path; }
const std::string& HttpRequest::query() const { return _query; }
const std::string& HttpRequest::version() const { return _version; }
const std::map<std::string, std::string>& HttpRequest::headers() const { return _headers; }
const std::map<std::string, std::string>& HttpRequest::cookies() const { return _cookies; }
const std::string& HttpRequest::body() const { return _body; }

std::string HttpRequest::header(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = _headers.find(toLower(key));
    if (it == _headers.end()) return "";
    return it->second;
}

std::string HttpRequest::cookie(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = _cookies.find(key);
    if (it == _cookies.end()) return "";
    return it->second;
}

std::string HttpRequest::trim(const std::string& s) {
    size_t b = 0;
    while (b < s.size() && std::isspace(static_cast<unsigned char>(s[b]))) b++;
    size_t e = s.size();
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) e--;
    return s.substr(b, e - b);
}

std::string HttpRequest::toLower(const std::string& s) {
    std::string r = s;
    for (size_t i = 0; i < r.size(); ++i)
        r[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(r[i])));
    return r;
}

HttpRequest::Method HttpRequest::parseMethod(const std::string& m) {
    if (m == "GET") return GET;
    if (m == "POST") return POST;
    if (m == "DELETE") return DELETE_;
    return UNKNOWN;
}

void HttpRequest::parseTarget() {
    _path = _target;
    _query.clear();
    size_t q = _target.find('?');
    if (q != std::string::npos) {
        _path = _target.substr(0, q);
        _query = _target.substr(q + 1);
    }
    if (_path.empty()) _path = "/";
}

void HttpRequest::parseCookies() {
    _cookies.clear();
    std::string c = header("cookie");
    if (c.empty()) return;

    // Cookie: a=b; c=d
    size_t i = 0;
    while (i < c.size()) {
        // skip spaces and separators
        while (i < c.size() && (c[i] == ' ' || c[i] == ';')) i++;
        if (i >= c.size()) break;

        size_t eq = c.find('=', i);
        if (eq == std::string::npos) break;

        std::string key = trim(c.substr(i, eq - i));
        size_t end = c.find(';', eq + 1);
        std::string val = (end == std::string::npos)
            ? trim(c.substr(eq + 1))
            : trim(c.substr(eq + 1, end - (eq + 1)));

        if (!key.empty()) _cookies[key] = val;
        if (end == std::string::npos) break;
        i = end + 1;
    }
}

bool HttpRequest::tryParseFrom(std::string& buffer) {
    reset();

    // Find end of headers
    size_t hdrEnd = buffer.find("\r\n\r\n");
    if (hdrEnd == std::string::npos) return false;

    std::string head = buffer.substr(0, hdrEnd);
    size_t consumed = hdrEnd + 4;

    // Parse start line + headers
    std::istringstream iss(head);
    std::string line;

    if (!std::getline(iss, line)) { _badRequest = true; _complete = true; buffer.erase(0, consumed); return true; }
    if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);

    std::istringstream sl(line);
    std::string m, t, v;
    if (!(sl >> m >> t >> v)) { _badRequest = true; _complete = true; buffer.erase(0, consumed); return true; }

    _method = parseMethod(m);
    _target = t;
    _version = v;

    // headers
    while (std::getline(iss, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
        if (line.empty()) break;

        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;
        std::string k = toLower(trim(line.substr(0, colon)));
        std::string val = trim(line.substr(colon + 1));
        _headers[k] = val;
    }

    parseTarget();
    parseCookies();

    // Body length
    size_t bodyLen = 0;
    std::string cl = header("content-length");
    if (!cl.empty()) bodyLen = static_cast<size_t>(std::strtoul(cl.c_str(), 0, 10));

    // Need more bytes?
    if (buffer.size() < consumed + bodyLen) return false;

    _body = buffer.substr(consumed, bodyLen);
    consumed += bodyLen;

    buffer.erase(0, consumed);
    _complete = true;
    return true;
}

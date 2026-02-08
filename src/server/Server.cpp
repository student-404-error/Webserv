#include "Server.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <cerrno>
#include <sstream>

static void fatal(const char* msg) {
    perror(msg);
    std::exit(1);
}

Server::Server(const std::vector<ServerConfig>& cfgs)
: _configs(cfgs), _listenFd(-1), _sidSeq(1) {
    if (_configs.empty()) {
        throw std::runtime_error("No server config provided");
    }

    // 우선 첫 번째 서버 블록의 첫 포트만 사용 (다중 포트는 추후 확장)
    const std::vector<int>& ports = _configs[0].getListenPorts();
    if (ports.empty()) {
        throw std::runtime_error("No listen port configured");
    }
    _port = ports[0];

    // 기본값 설정 (TODO: config에서 받아오도록 확장)
    _maxConnections = 1024;
    _idleTimeoutSec = 15;

    _listenFd = createListenSocket(_port);
    setNonBlocking(_listenFd);

    pollfd p;
    p.fd = _listenFd;
    p.events = POLLIN;
    p.revents = 0;
    _pfds.push_back(p);

    std::cout << "Listening on port " << _port << "\n";
}

Server::~Server() {
    for (std::map<int, Connection*>::iterator it = _conns.begin(); it != _conns.end(); ++it)
        delete it->second;
    _conns.clear();

    if (_listenFd != -1) ::close(_listenFd);
}

int Server::createListenSocket(int port) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) fatal("socket");

    int opt = 1;
    if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        fatal("setsockopt");

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (::bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) fatal("bind");
    if (::listen(fd, 128) < 0) fatal("listen");
    return fd;
}

void Server::setNonBlocking(int fd) {
    int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags < 0) fatal("fcntl(F_GETFL)");
    if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) fatal("fcntl(F_SETFL)");
}

void Server::run() {
    while (true) {
        sweepTimeouts();

        int ready = ::poll(&_pfds[0], _pfds.size(), 1000); // 1s tick
        if (ready < 0) {
            if (errno == EINTR) continue;
            fatal("poll");
        }
        if (ready == 0) continue;

        for (size_t i = 0; i < _pfds.size(); /* increment inside */) {
            if (_pfds[i].revents == 0) { ++i; continue; }

            if (i == 0) {
                if (_pfds[i].revents & POLLIN) acceptLoop();
                _pfds[i].revents = 0;
                ++i;
                continue;
            }

            handleClientEvent(i);
            // handleClientEvent may remove current index; safest is to just continue without ++i if removed
            // We'll detect by checking i within bounds and revents reset.
            if (i < _pfds.size() && _pfds[i].revents == 0) ++i;
        }

        for (size_t k = 0; k < _pfds.size(); ++k) _pfds[k].revents = 0;
    }
}

void Server::acceptLoop() {
    while (true) {
        int cfd = ::accept(_listenFd, NULL, NULL);
        if (cfd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            perror("accept");
            return;
        }

        if ((int)_conns.size() >= _maxConnections) {
            ::close(cfd);
            continue;
        }

        setNonBlocking(cfd);

        Connection* conn = new Connection(cfd);
        _conns[cfd] = conn;

        pollfd p;
        p.fd = cfd;
        p.events = POLLIN; // start with read
        p.revents = 0;
        _pfds.push_back(p);

        std::cout << "Accepted fd=" << cfd << "\n";
    }
}

void Server::handleClientEvent(size_t idx) {
    int fd = _pfds[idx].fd;
    Connection* conn = 0;

    std::map<int, Connection*>::iterator it = _conns.find(fd);
    if (it == _conns.end()) {
        removeConn(fd);
        return;
    }
    conn = it->second;

    short ev = _pfds[idx].revents;

    if (ev & (POLLERR | POLLHUP | POLLNVAL)) {
        removeConn(fd);
        return;
    }

    if (ev & POLLIN) {
        if (!conn->onReadable()) { removeConn(fd); return; }

        // Try parse requests as long as possible (pipelining-friendly)
        while (true) {
            HTTPRequest req;
            std::string& in = conn->inBuf();

            // HTTPRequest::appendData를 사용하여 파싱 시도
            bool complete = req.appendData(in);
            if (!complete) break; // need more bytes

            if (req.hasError()) {
                HTTPResponse resp;
                resp.setStatus(400);
                resp.setBody("400 Bad Request\n");
                std::string bytes = resp.toString();
                conn->queueWrite(bytes);
                conn->closeAfterWrite();
                break;
            }

            // 파싱 성공 - 처리된 데이터는 버퍼에서 제거해야 함
            // 간단하게 전체 버퍼를 클리어 (단일 요청 처리)
            in.clear();
            
            onRequest(fd, req);
            // if handler queued write and wants close, we may stop parsing more
            if (conn->shouldCloseAfterWrite()) break;
        }
    }

    if (ev & POLLOUT) {
        if (!conn->onWritable()) { removeConn(fd); return; }
    }

    updatePollEventsFor(fd);
    _pfds[idx].revents = 0;
}

void Server::updatePollEventsFor(int fd) {
    Connection* conn = _conns[fd];
    for (size_t i = 1; i < _pfds.size(); ++i) {
        if (_pfds[i].fd == fd) {
            short e = POLLIN;
            if (conn->hasPendingWrite()) e |= POLLOUT;
            _pfds[i].events = e;
            return;
        }
    }
}

void Server::removeConn(int fd) {
    std::map<int, Connection*>::iterator it = _conns.find(fd);
    if (it != _conns.end()) {
        delete it->second;
        _conns.erase(it);
    }
    for (size_t i = 1; i < _pfds.size(); ++i) {
        if (_pfds[i].fd == fd) {
            _pfds.erase(_pfds.begin() + i);
            break;
        }
    }
    std::cout << "Closed fd=" << fd << "\n";
}

void Server::sweepTimeouts() {
    std::time_t now = std::time(NULL);
    std::vector<int> toClose;

    for (std::map<int, Connection*>::iterator it = _conns.begin(); it != _conns.end(); ++it) {
        Connection* c = it->second;
        if ((int)(now - c->lastActive()) > _idleTimeoutSec) {
            toClose.push_back(it->first);
        }
    }
    for (size_t i = 0; i < toClose.size(); ++i) removeConn(toClose[i]);

    // (optional) session cleanup (very simple)
    for (std::map<std::string, Session>::iterator sit = _sessions.begin(); sit != _sessions.end(); ) {
        if ((int)(now - sit->second.lastSeen) > 300) { // 5 minutes
            std::map<std::string, Session>::iterator kill = sit++;
            _sessions.erase(kill);
        } else {
            ++sit;
        }
    }
}

std::string Server::newSessionId() {
    // Simple, non-crypto SID (bonus demo용)
    std::ostringstream oss;
    oss << std::time(NULL) << "-" << (_sidSeq++);
    return oss.str();
}

Server::Session& Server::getOrCreateSession(const HTTPRequest& req, HTTPResponse& resp) {
    // HTTPRequest에 cookie 메서드가 없으므로 헤더에서 직접 파싱
    std::string cookieHeader;
    const std::map<std::string, std::string>& headers = req.getHeaders();
    std::map<std::string, std::string>::const_iterator it = headers.find("cookie");
    if (it != headers.end()) {
        cookieHeader = it->second;
    }
    
    // 간단한 쿠키 파싱 (sid=value 형태)
    std::string sid;
    if (!cookieHeader.empty()) {
        size_t pos = cookieHeader.find("sid=");
        if (pos != std::string::npos) {
            size_t start = pos + 4;
            size_t end = cookieHeader.find(";", start);
            if (end == std::string::npos) end = cookieHeader.length();
            sid = cookieHeader.substr(start, end - start);
        }
    }
    
    if (sid.empty() || _sessions.find(sid) == _sessions.end()) {
        sid = newSessionId();
        _sessions[sid] = Session();
        resp.setHeader("Set-Cookie", "sid=" + sid + "; Path=/; HttpOnly");
    }
    Session& s = _sessions[sid];
    s.lastSeen = std::time(NULL);
    return s;
}

void Server::onRequest(int fd, const HTTPRequest& req) {
    Connection* conn = _conns[fd];

    // keep-alive decision (simple)
    bool keepAlive = true;
    const std::map<std::string, std::string>& headers = req.getHeaders();
    std::map<std::string, std::string>::const_iterator connIt = headers.find("connection");
    if (connIt != headers.end()) {
        std::string connHdr = connIt->second;
        if (connHdr == "close" || connHdr == "Close") {
            keepAlive = false;
        }
    }

    HTTPResponse resp;

    // URI에서 경로 추출 (쿼리 스트링 제거)
    std::string uri = req.getURI();
    std::string path = uri;
    size_t qpos = uri.find('?');
    if (qpos != std::string::npos) {
        path = uri.substr(0, qpos);
    }

    // ---- Simple routes (session demo) ----
    if (path == "/") {
        resp.setBody("Hello from webserv skeleton\nTry /counter\n");
    }
    else if (path == "/counter") {
        Session& s = getOrCreateSession(req, resp);
        s.counter++;

        std::ostringstream body;
        body << "session counter = " << s.counter << "\n";
        resp.setBody(body.str());
    }
    else if (path == "/logout") {
        // expire cookie (very simple)
        resp.setHeader("Set-Cookie", "sid=deleted; Path=/; Max-Age=0");
        resp.setBody("logged out\n");
    }
    else {
        resp.setStatus(404);
        resp.setBody("404 Not Found\n");
    }

    // Connection 헤더 설정
    if (keepAlive) {
        resp.setHeader("Connection", "keep-alive");
    } else {
        resp.setHeader("Connection", "close");
    }

    std::string bytes = resp.toString();
    conn->queueWrite(bytes);

    if (!keepAlive) conn->closeAfterWrite();
}

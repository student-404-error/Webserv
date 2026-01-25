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

Server::Server(const Config& cfg)
: _cfg(cfg), _listenFd(-1), _sidSeq(1) {
    _listenFd = createListenSocket(_cfg.port);
    setNonBlocking(_listenFd);

    pollfd p;
    p.fd = _listenFd;
    p.events = POLLIN;
    p.revents = 0;
    _pfds.push_back(p);

    std::cout << "Listening on port " << _cfg.port << "\n";
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

        if ((int)_conns.size() >= _cfg.maxConnections) {
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
            HttpRequest req;
            std::string& in = conn->inBuf();

            bool got = req.tryParseFrom(in);
            if (!got) break; // need more bytes

            if (req.badRequest()) {
                HttpResponse resp;
                resp.setStatus(400, "Bad Request");
                resp.setBody("400 Bad Request\n");
                std::string bytes = resp.toBytes(false);
                conn->queueWrite(bytes);
                conn->closeAfterWrite();
                break;
            }

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
        if ((int)(now - c->lastActive()) > _cfg.idleTimeoutSec) {
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

Server::Session& Server::getOrCreateSession(const HttpRequest& req, HttpResponse& resp) {
    std::string sid = req.cookie("sid");
    if (sid.empty() || _sessions.find(sid) == _sessions.end()) {
        sid = newSessionId();
        _sessions[sid] = Session();
        resp.addSetCookie("sid=" + sid + "; Path=/; HttpOnly");
    }
    Session& s = _sessions[sid];
    s.lastSeen = std::time(NULL);
    return s;
}

void Server::onRequest(int fd, const HttpRequest& req) {
    Connection* conn = _conns[fd];

    // keep-alive decision (simple)
    bool keepAlive = true;
    std::string connHdr = req.header("connection");
    if (!connHdr.empty() && (connHdr == "close" || connHdr == "Close")) keepAlive = false;

    HttpResponse resp;

    // ---- Simple routes (session demo) ----
    if (req.path() == "/") {
        resp.setBody("Hello from webserv skeleton\nTry /counter\n");
    }
    else if (req.path() == "/counter") {
        Session& s = getOrCreateSession(req, resp);
        s.counter++;

        std::ostringstream body;
        body << "session counter = " << s.counter << "\n";
        resp.setBody(body.str());
    }
    else if (req.path() == "/logout") {
        // expire cookie (very simple)
        resp.addSetCookie("sid=deleted; Path=/; Max-Age=0");
        resp.setBody("logged out\n");
    }
    else {
        resp.setStatus(404, "Not Found");
        resp.setBody("404 Not Found\n");
    }

    std::string bytes = resp.toBytes(keepAlive);
    conn->queueWrite(bytes);

    if (!keepAlive) conn->closeAfterWrite();
}

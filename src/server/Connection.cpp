#include "Connection.hpp"
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>

Connection::Connection(int fd)
: _fd(fd), _state(READING), _closeAfterWrite(false), _lastActive(std::time(NULL)) {}

Connection::~Connection() {
    if (_fd != -1) ::close(_fd);
}

int Connection::fd() const { return _fd; }
Connection::State Connection::state() const { return _state; }

std::string& Connection::inBuf() { return _in; }
const std::string& Connection::inBuf() const { return _in; }

void Connection::queueWrite(const std::string& bytes) {
    _out.append(bytes);
    if (!_out.empty()) _state = WRITING;
}

bool Connection::hasPendingWrite() const { return !_out.empty(); }

void Connection::closeAfterWrite() { _closeAfterWrite = true; }
bool Connection::shouldCloseAfterWrite() const { return _closeAfterWrite; }

void Connection::touch() { _lastActive = std::time(NULL); }
std::time_t Connection::lastActive() const { return _lastActive; }

bool Connection::onReadable() {
    char buf[8192];
    while (true) {
        ssize_t n = ::recv(_fd, buf, sizeof(buf), 0);
        if (n > 0) {
            _in.append(buf, n);
            touch();
            continue; // more may be available
        }
        if (n == 0) {
            // peer closed (FIN)
            return false;
        }
        // n < 0
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return true; // no more for now
        }
        // error
        return false;
    }
}

bool Connection::onWritable() {
    while (!_out.empty()) {
        ssize_t n = ::send(_fd, _out.data(), _out.size(), 0);
        if (n > 0) {
            _out.erase(0, n);
            touch();
            continue;
        }
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return true; // try later
        }
        // error
        return false;
    }

    // finished writing
    if (_closeAfterWrite) return false;
    _state = READING;
    return true;
}
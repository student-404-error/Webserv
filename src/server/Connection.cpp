#include "Connection.hpp"
#include <unistd.h>
#include <sys/socket.h>

Connection::Connection(int fd)
: _fd(fd), _state(READING), _outPos(0), _closeAfterWrite(false),
  _lastActive(std::time(NULL)), _requestsHandled(0) {}

Connection::~Connection() {
    if (_fd != -1) ::close(_fd);
}

int Connection::fd() const { return _fd; }
Connection::State Connection::state() const { return _state; }

std::string& Connection::inBuf() { return _in; }
const std::string& Connection::inBuf() const { return _in; }

void Connection::queueWrite(const std::string& bytes) {
    // Clear old data if we've already sent everything
    if (_outPos > 0 && _outPos == _out.size()) {
        _out.clear();
        _outPos = 0;
    }
    _out.append(bytes);
    if (!_out.empty()) _state = WRITING;
}

bool Connection::hasPendingWrite() const { return _outPos < _out.size(); }

void Connection::closeAfterWrite() {
    _closeAfterWrite = true;
    // If there is nothing left to write, close immediately so the connection
    // does not depend on a future onWritable() call that may never happen.
    if (!hasPendingWrite() && _fd != -1) {
        ::close(_fd);
        _fd = -1;
    }
}
bool Connection::shouldCloseAfterWrite() const { return _closeAfterWrite; }

void Connection::touch() { _lastActive = std::time(NULL); }
std::time_t Connection::lastActive() const { return _lastActive; }

void Connection::incRequestCount() { ++_requestsHandled; }
int Connection::requestCount() const { return _requestsHandled; }

bool Connection::onReadable() {
    // Must be large enough to return 413 for body-limit violations instead of
    // dropping the connection before request validation.
    static const size_t MAX_INPUT_SIZE = (10 * 1024 * 1024) + (64 * 1024);

    char buf[8192];
    ssize_t n = ::recv(_fd, buf, sizeof(buf), 0);
    if (n > 0) {
        // Check for buffer overflow before appending
        if (_in.size() + static_cast<size_t>(n) > MAX_INPUT_SIZE) {
            // Input buffer limit exceeded - close connection
            return false;
        }
        _in.append(buf, n);
        touch();
        return true;
    }
    if (n == 0) {
        // peer closed (FIN)
        return false;
    }
    // n < 0: do not branch on errno after recv
    return true;
}

bool Connection::onWritable() {
    if (_outPos >= _out.size()) {
        _out.clear();
        _outPos = 0;
        if (_closeAfterWrite) return false;
        _state = READING;
        return true;
    }

    int flags = 0;
#ifdef MSG_NOSIGNAL
    flags |= MSG_NOSIGNAL;
#endif

    ssize_t n = ::send(_fd, _out.data() + _outPos, _out.size() - _outPos, flags);
    if (n > 0) {
        _outPos += static_cast<size_t>(n);
        touch();
    }
    // n < 0: do not branch on errno after send

    if (_outPos < _out.size())
        return true;

    _out.clear();
    _outPos = 0;
    if (_closeAfterWrite) return false;
    _state = READING;
    return true;
}

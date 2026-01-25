#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <ctime>

class Connection {
public:
    enum State {
        READING,
        WRITING,
        CLOSING
    };

    explicit Connection(int fd);
    ~Connection();

    int fd() const;
    State state() const;

    // I/O (non-blocking)
    // returns false if connection should be closed now
    bool onReadable();
    bool onWritable();

    std::string& inBuf();
    const std::string& inBuf() const;

    void queueWrite(const std::string& bytes);
    bool hasPendingWrite() const;

    void closeAfterWrite();
    bool shouldCloseAfterWrite() const;

    void touch();
    std::time_t lastActive() const;

private:
    int _fd;
    State _state;

    std::string _in;
    std::string _out;

    bool _closeAfterWrite;
    std::time_t _lastActive;

    Connection(const Connection&);
    Connection& operator=(const Connection&);
};

#endif

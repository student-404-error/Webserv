#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <map>
#include <string>
#include <ctime>
#include <poll.h>

#include "Connection.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ServerConfig.hpp"

class Server {
public:
    // explicit: 암묵적 변환을 막아 잘못된 생성 호출을 방지
    explicit Server(const std::vector<ServerConfig>& cfgs);
    ~Server();

    void run();

private:
    std::vector<ServerConfig> _configs;      // 모든 server 블록 설정을 저장
    int _port;
    int _maxConnections;
    int _idleTimeoutSec;
    int _listenFd;

    std::vector<pollfd> _pfds;               // [0] is listen fd
    std::map<int, Connection*> _conns;       // fd -> Connection*

    // simple in-memory session store
    struct Session {
        int counter;
        std::time_t lastSeen;
        Session() : counter(0), lastSeen(std::time(NULL)) {}
    };
    std::map<std::string, Session> _sessions;
    unsigned long _sidSeq;

private:
    int createListenSocket(int port);
    void setNonBlocking(int fd);

    void acceptLoop();
    void handleClientEvent(size_t idx);

    void updatePollEventsFor(int fd);
    void removeConn(int fd);

    void sweepTimeouts();

    // request/response flow
    void onRequest(int fd, const HTTPRequest& req);

    // session helpers
    std::string newSessionId();
    Session& getOrCreateSession(const HTTPRequest& req, HTTPResponse& resp);
};

#endif

#include <iostream>
#include <vector>
#include "Config.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Server.hpp"

int main(int argc, char* argv[])
{
    std::cout << "=== Webserv Test ===" << std::endl;

    // 1. Config 파싱 테스트
    std::cout << "\n[1] Config Parsing Test" << std::endl;
    try {
        if (argc > 1) {
            Config config(argv[1]);
            config.configParse();
            std::cout << "Config parsing successful!" << std::endl;
        } else {
            std::cout << "No config file provided. Skipping config test." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Config error: " << e.what() << std::endl;
    }

    // 2. HTTP Request 파싱 테스트
    std::cout << "\n[2] HTTP Request Parsing Test" << std::endl;
    HttpRequest req;
    
    std::string testRequest = 
        "GET /index.html HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: test-client\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, World!";
    
    if (req.appendData(testRequest)) {
        std::cout << "Request parsing complete!" << std::endl;
        std::cout << "Method: " << req.getMethod() << std::endl;
        std::cout << "URI: " << req.getURI() << std::endl;
        std::cout << "Version: " << req.getVersion() << std::endl;
        std::cout << "Body: " << req.getBody() << std::endl;
    }

    // 3. HTTP Response 생성 테스트
    std::cout << "\n[3] HTTP Response Generation Test" << std::endl;
    HttpResponse resp;
    resp.setStatus(200);
    resp.setContentType("text/html");
    resp.setBody("<html><body><h1>Hello, Webserv!</h1></body></html>");
    
    std::cout << "Generated Response:" << std::endl;
    std::cout << resp.toString() << std::endl;

    // 4. 서버 실행 (옵션)
    std::cout << "\n[4] Starting HTTP Server" << std::endl;
    std::cout << "Server will listen on port 8080" << std::endl;
    std::cout << "Visit http://localhost:8080/ in your browser" << std::endl;
    std::cout << "Press Ctrl+C to stop\n" << std::endl;

    try {
        // 테스트용 ServerConfig 생성
        ServerConfig serverConfig;
        serverConfig.addListenPort(8080);

        std::vector<ServerConfig> servers;
        servers.push_back(serverConfig);

        Server server(servers);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

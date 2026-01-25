#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

# include <string>
# include <map>
# include <vector>

class HttpResponse {
public:
    HttpResponse();

    void setStatus(int code, const std::string& reason);
    void setBody(const std::string& body);
    void addHeader(const std::string& key, const std::string& value);

    // cookie helper
    void addSetCookie(const std::string& cookieLine);
    // full: "sid=..."; Path=/; HttpOnly;

    std::string toBytes(bool keepAlive) const;

private:
    int _code;
    std::string _reason;
    std::map<std::string, std::string> _headers;
    std::vector<std::string> _setCookies;
    std::string _body;

    static std::string itoa(int n);
};

#endif
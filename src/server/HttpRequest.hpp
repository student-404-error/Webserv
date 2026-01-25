#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <string>
# include <map>

class HttpRequest
{
public:
    enum Method { GET, POST, DELETE_, UNKNOWN };

    HttpRequest();

    bool isComplete() const;
    void reset();

    Method method() const;
    const std::string& target() const;
    const std::string& path() const;
    const std::string& query() const;
    const std::string& version() const;

    const std::map<std::string, std::string>& headers() const;
    std::string header(const std::string& key) const;

    const std::map<std::string, std::string>& cookies() const;
    std::string cookie(const std::string& key) const;

    const std::string& body() const;

    // Parse from a buffer. If a full request is available, consume it from buffer and return true.
    // If not enough data, return false (buffer unchanged except possibly some internal checks).
    // On bad request, sets _badRequest=true and returns true (so caller can respond 400).
    bool tryParseFrom(std::string& buffer);

    bool badRequest() const;

private:
    Method _method;
    std::string _target;
    std::string _path;
    std::string _query;
    std::string _version;
    std::map<std::string, std::string> _headers;
    std::map<std::string, std::string> _cookies;
    std::string _body;

    bool _complete;
    bool _badRequest;

    static std::string trim(const std::string& s);
    static std::string toLower(const std::string& s);
    static Method parseMethod(const std::string& m);

    void parseTarget();
    void parseCookies();
};

#endif

#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "LocationConfig.hpp"

class RequestHandler {
public:
    virtual ~RequestHandler() {}
    virtual HttpResponse handle(const HttpRequest& request,
                                const LocationConfig& location) = 0;
};

#endif

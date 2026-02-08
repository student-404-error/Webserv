/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaoh <jaoh@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 17:52:50 by jaoh              #+#    #+#             */
/*   Updated: 2026/02/08 15:40:47 by jaoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef POSTHANDLER_HPP
#define POSTHANDLER_HPP

#include "RequestHandler.hpp"
#include <map>

class POSTHandler : public RequestHandler {
public:
    POSTHandler(size_t maxBodySizeBytes);
    virtual HttpResponse handle(const HttpRequest& request,
                                const LocationConfig& location);

private:
    bool checkBodySize(const HttpRequest& request, HttpResponse& res) const;

    // Content-Type 분기
    HttpResponse handleFormUrlEncoded(const HttpRequest& request,
                                      const LocationConfig& location);
    HttpResponse handleMultipart(const HttpRequest& request,
                                 const LocationConfig& location,
                                 const std::string& boundary);

    // parsers
    std::map<std::string, std::string> parseUrlEncoded(const std::string& body) const;
    bool parseMultipart(const std::string& body,
                        const std::string& boundary,
                        /* out */ std::vector< std::map<std::string,std::string> >& parts) const;

    std::string urlDecode(const std::string& s) const;
    std::string trim(const std::string& s) const;
    std::string toLower(const std::string& s) const;

private:
    size_t maxBodySize;
};

#endif

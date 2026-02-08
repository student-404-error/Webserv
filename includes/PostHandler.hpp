/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Posthandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaoh <jaoh@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 17:52:50 by jaoh              #+#    #+#             */
/*   Updated: 2026/02/03 17:52:58 by jaoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef POSTHANDLER_HPP
#define POSTHANDLER_HPP

#include "RequestHandler.hpp"
#include <map>

class POSTHandler : public RequestHandler {
public:
    POSTHandler(size_t maxBodySizeBytes);
    virtual HTTPResponse handle(const HTTPRequest& request,
                                const LocationConfig& location);

private:
    bool checkBodySize(const HTTPRequest& request, HTTPResponse& res) const;

    // Content-Type 분기
    HTTPResponse handleFormUrlEncoded(const HTTPRequest& request,
                                      const LocationConfig& location);
    HTTPResponse handleMultipart(const HTTPRequest& request,
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

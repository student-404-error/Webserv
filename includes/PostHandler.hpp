/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaoh <jaoh@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 17:52:50 by jaoh              #+#    #+#             */
/*   Updated: 2026/02/12 01:04:53 by jaoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef POSTHANDLER_HPP
#define POSTHANDLER_HPP

#include "RequestHandler.hpp"
#include <map>
#include <vector>

class POSTHandler : public RequestHandler {
public:
    POSTHandler(size_t maxBodySizeBytes);
    virtual HttpResponse handle(const HttpRequest& request,
                                const LocationConfig& location);

private:
    // Body size 검증
    bool checkBodySize(const HttpRequest& request, HttpResponse& res) const;

    // Content-Type 분기
    HttpResponse handleFormUrlEncoded(const HttpRequest& request,
                                      const LocationConfig& location);
    HttpResponse handleMultipart(const HttpRequest& request,
                                 const LocationConfig& location,
                                 const std::string& boundary);

    // 파일 저장 관련
    bool saveFile(const std::string& path, const std::string& data) const;
    std::string sanitizeFilename(const std::string& name) const;
    std::string generateUniqueFilename(const std::string& dir,
                                      const std::string& name) const;
    bool isPathSafe(const std::string& fullPath,
                   const std::string& baseDir) const;

    // Parsers
    std::map<std::string, std::string> parseUrlEncoded(const std::string& body) const;
    bool parseMultipart(const std::string& body,
                        const std::string& boundary,
                        std::vector< std::map<std::string,std::string> >& parts) const;

    // Utils
    std::string urlDecode(const std::string& s) const;
    std::string trim(const std::string& s) const;
    std::string toLower(const std::string& s) const;

private:
    size_t maxBodySize;
};

#endif

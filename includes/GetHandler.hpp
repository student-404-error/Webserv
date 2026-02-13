/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaoh <jaoh@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/25 16:30:07 by jaoh              #+#    #+#             */
/*   Updated: 2026/02/13 16:03:39 by jaoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GETHANDLER_HPP
#define GETHANDLER_HPP

#include "RequestHandler.hpp"
#include <sys/stat.h>

class GETHandler : public RequestHandler {
public:
    virtual HttpResponse handle(const HttpRequest& request,
                                const LocationConfig& location);

private:
    // 디렉토리 처리
    HttpResponse handleDirectory(const std::string& path,
                                 const std::string& uri,
                                 const LocationConfig& location);
    
    // 파일 처리
    HttpResponse handleFile(const std::string& path,
                           const struct stat& st);

    // 경로 빌드 및 검증
    std::string buildPath(const std::string& uri,
                          const LocationConfig& location) const;
    bool isPathSafe(const std::string& path) const;
    std::string urlDecode(const std::string& s) const;

    // 파일 읽기
    std::string readFile(const std::string& path) const;
    
    // autoindex HTML 생성
    std::string generateAutoIndex(const std::string& path,
                                  const std::string& uri) const;
    
    // MIME 타입 결정
    std::string getMimeType(const std::string& path) const;
};

#endif

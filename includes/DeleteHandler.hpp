/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaoh <jaoh@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 17:53:24 by jaoh              #+#    #+#             */
/*   Updated: 2026/02/12 01:19:38 by jaoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DELETEHANDLER_HPP
#define DELETEHANDLER_HPP

#include "RequestHandler.hpp"
#include <string>

// 향상된 DELETE 핸들러
// - 보안 강화 (디렉토리 삭제 방지, path traversal 방어)
// - 더 명확한 에러 메시지
// - 삭제 전 검증 강화
// - 권한 확인
class DELETEHandler : public RequestHandler {
public:
    virtual HttpResponse handle(const HttpRequest& request,
                                const LocationConfig& location);

private:
    // 경로 빌드 및 검증
    std::string buildPath(const std::string& uri,
                          const LocationConfig& location) const;
    
    // 파일 타입 체크
    bool isRegularFile(const std::string& path) const;
    bool isDirectory(const std::string& path) const;
    bool pathExists(const std::string& path) const;
    
    // 삭제 가능 여부 검증
    bool canDelete(const std::string& path) const;
    
    // 보안 검증
    bool isPathSafe(const std::string& path) const;
    
    // 실제 삭제 수행
    bool deleteFile(const std::string& path) const;
};

#endif

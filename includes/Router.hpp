/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaoh <jaoh@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/23 16:28:35 by jaoh              #+#    #+#             */
/*   Updated: 2026/02/13 12:39:10 by jaoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string>
#include <vector>
#include <set>
#include <map>

// 하나의 location 블록에 대한 완전한 설정 정보
struct LocationConfig {
    // 기본 경로 설정
    std::string path;               // 요청 URI prefix, 예) /images
    std::string root;               // 실제 파일 시스템 경로, 예) /var/www/img
    std::set<std::string> methods;  // 허용 HTTP 메서드 목록, 예) GET, POST
    
    // 리다이렉트 설정
    bool redirect;                  // 리다이렉트 여부
    int redirectCode;               // 리다이렉트 상태 코드 (301, 302, 307, 308)
    std::string redirectTarget;     // 리다이렉트 대상 경로
    
    // autoindex 설정 (디렉토리 리스팅)
    bool autoindex;                 // autoindex 활성화 여부
    
    // index 파일 설정
    std::vector<std::string> indexFiles; // 기본 index 파일들 (index.html, index.php 등)
    
    // 파일 업로드 설정
    bool uploadEnabled;             // 업로드 허용 여부
    std::string uploadDir;          // 업로드 파일 저장 경로
    
    // CGI 설정
    bool cgiEnabled;                // CGI 활성화 여부
    std::map<std::string, std::string> cgiInterpreters; // 확장자별 인터프리터
    // 예: {".php": "/usr/bin/php-cgi", ".py": "/usr/bin/python3"}
    
    // 에러 페이지 설정 (선택적, 서버 전역 설정이 우선일 수 있음)
    std::map<int, std::string> errorPages; // 상태 코드별 에러 페이지 경로
    
    // 생성자: 기본값 설정
    LocationConfig() 
        : redirect(false),
          redirectCode(301),
          autoindex(false),
          uploadEnabled(false),
          cgiEnabled(false) {
        // 기본 index 파일
        indexFiles.push_back("index.html");
        indexFiles.push_back("index.htm");
    }
};

// Router
// - 여러 LocationConfig를 관리하고
// - 들어온 요청의 URI에 대해 어떤 location을 사용할지 결정하는 책임을 가진다.
class Router {
public:
    Router();

    void addLocation(const LocationConfig& loc);

    // URI에 가장 잘 매칭되는 location 반환 (가장 긴 prefix)
    const LocationConfig* match(const std::string& uri) const;
    
    // 해당 location에서 HTTP method가 허용되는지 확인
    bool isMethodAllowed(const LocationConfig* loc, const std::string& method) const;

private:
    std::vector<LocationConfig> locations;
};

#endif
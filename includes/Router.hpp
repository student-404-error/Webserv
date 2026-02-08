/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaoh <jaoh@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/23 16:28:35 by jaoh              #+#    #+#             */
/*   Updated: 2026/02/05 16:58:16 by jaoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string>
#include <vector>
#include <set>

// 하나의 location 블록에 대한 설정 정보
//   ex)
//   location /images {
//       root /var/www/img;
//       methods GET POST;
//   }
struct LocationConfig {
    std::string path;               // 요청 URI prefix, 예) /images
    std::string root;               // 실제 파일 시스템 경로, 예) /var/www/img
    std::set<std::string> methods;  // 허용 HTTP 메서드 목록, 예) GET, POST
    bool redirect;                  // 리다이렉트 여부
    std::string redirectTarget;     // 리다이렉트 대상 경로, 예) /new-path
};

// Router
// - 여러 LocationConfig를 관리하고
// - 들어온 요청의 URI에 대해 어떤 location을 사용할지 결정하는 책임을 가진다.
class Router {
public:
    Router();

    void addLocation(const LocationConfig& loc);

    const LocationConfig* match(const std::string& uri) const;
    bool isMethodAllowed(const LocationConfig* loc, const std::string& method) const;

private:
    std::vector<LocationConfig> locations;
};

#endif

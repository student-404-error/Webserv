/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaoh <jaoh@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/23 16:28:47 by jaoh              #+#    #+#             */
/*   Updated: 2026/02/16 11:49:58 by jaoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"
#include <algorithm>

// Router
// - 여러 LocationConfig를 담고 있고
// - URI에 가장 잘 매칭되는(location.path가 가장 긴 prefix) location을 찾아주는 역할
Router::Router() {}

// 설정 파일 등에서 읽어온 location 설정을 내부 벡터에 추가
void Router::addLocation(const LocationConfig& loc) {
    locations.push_back(loc);
}

/*
** Prefix matching
**   예) /images/cat.png → /images
** 여러 location 중에서 uri의 prefix로 가장 길게 매칭되는 location을 선택한다.
*/
const LocationConfig* Router::match(const std::string& uri) const {
    const LocationConfig* best = NULL;
    size_t bestLen = 0;

    // 모든 location을 순회하면서 prefix 매칭 확인
    for (size_t i = 0; i < locations.size(); i++) {
        const std::string& path = locations[i].getPath();

        // uri가 path로 시작하면(prefix) 후보로 본다.
        if (uri.compare(0, path.size(), path) == 0) {
            // 가장 긴 path를 가진 location을 선택 (more specific)
            if (path.size() > bestLen) {
                best = &locations[i];
                bestLen = path.size();
            }
        }
    }
    return best;
}

// LocationConfig의 허용 메서드 목록에 현재 HTTP method가 포함되어 있는지 확인
// 우선순위: allow_methods > methods(legacy fallback)
bool Router::isMethodAllowed(const LocationConfig* loc,
                             const std::string& method) const {
    if (!loc)
        return false;

    if (method != "GET" && method != "POST" && method != "DELETE")
        return false;

    const std::vector<std::string>* allowed = NULL;
    if (loc->hasAllowMethods()) {
        allowed = &loc->getAllowMethods();
    } else if (loc->hasMethods()) {
        allowed = &loc->getMethods();
    } else {
        return false;
    }

    if (allowed->empty())
        return false;

    // New config prefers allow_methods. Legacy methods is kept as fallback.
    return std::find(allowed->begin(), allowed->end(), method) != allowed->end();
}

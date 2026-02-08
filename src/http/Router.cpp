/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaoh <jaoh@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/23 16:28:47 by jaoh              #+#    #+#             */
/*   Updated: 2026/02/08 14:42:22 by jaoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"

Router::Router() {}

void Router::addLocation(const LocationConfig& loc) {
    locations.push_back(loc);
}

/*
** Prefix matching
** /images/cat.png → /images
*/
const LocationConfig* Router::match(const std::string& uri) const {
    const LocationConfig* best = NULL;
    size_t bestLen = 0;

    for (size_t i = 0; i < locations.size(); i++) {
        const std::string& path = locations[i].path;

        if (uri.compare(0, path.size(), path) == 0) {
            if (path.size() > bestLen) {
                best = &locations[i];
                bestLen = path.size();
            }
        }
    }
    return best;
}

bool Router::isMethodAllowed(const LocationConfig* loc,
                             const std::string& method) const {
    if (!loc)
        return false;
    return loc->methods.count(method) > 0;
}

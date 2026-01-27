/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 17:31:47 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/01/27 18:22:22 by princessj        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
#include "LocationConfig.hpp" // 헤더에 "LocationConfig라는 타입이 있다"만 알고 내부 구조 모름 (전방 선언)

ServerConfig::ServerConfig(void) {}

ServerConfig::~ServerConfig(void) {}

void	ServerConfig::parseDirective(const std::vector<Token> &tokens, size_t &i) {}

void	ServerConfig::addLocation(const LocationConfig &location) {}

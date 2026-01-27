/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 17:31:37 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/01/27 18:06:01 by princessj        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"

LocationConfig::LocationConfig(const std::string &path) : _path(path), _autoindex(false) {}

LocationConfig::~LocationConfig() {}

void	LocationConfig::parseDirective(const std::vector<Token> &tokens, size_t &i) {}

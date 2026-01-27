/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 17:31:37 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/01/27 21:33:55 by princessj        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"

LocationConfig::LocationConfig(const std::string &path) : _path(path), _autoindex(false) {}

LocationConfig::~LocationConfig() {}

void	LocationConfig::handleRoot(const std::vector<Token> &tokens, size_t &i)
{
	if ((i + 1) > tokens.size())
		throw std::runtime_error("Error: Root requires a path");
	
	const Token	&pathToken = tokens[i + 1];

	if (pathToken.type != TOKEN_WORD)
		throw std::runtime_error("Error: Invalid root path");
	
	this->_root = pathToken.value;

	i += 2;
	if (tokens[i].type != TOKEN_SEMICOLON)
		throw std::runtime_error("Error: Missing ';' after root directive");
	
	i++;
}

void	LocationConfig::handleAutoindex(const std::vector<Token> &tokens, size_t &i)
{
	if ((i + 1) >= tokens.size())
		throw std::runtime_error("Error: Autoindex requires on or off");
	
	const Token	&valueToken = tokens[i + 1];
	
	if (valueToken.type != TOKEN_WORD)
		throw std::runtime_error("Error: Invalid autoindex value");
	
	if (valueToken.value == "on")
		this->_autoindex = true;
	else if (valueToken.value == "off")
		this->_autoindex = false;
	else
		throw std::runtime_error("Error: Autoindex must be 'on' or 'off'");
	
	i += 2;
	if (tokens[i].type != TOKEN_SEMICOLON)
		throw std::runtime_error("Error: Missing ';' after autoindex directive");
	
	i++;
}

void	LocationConfig::parseDirective(const std::vector<Token> &tokens, size_t &i)
{
	const std::string	&field = tokens[i].value;

	if (field == "root")
		handleRoot(tokens, i);
	else if (field == "autoindex")
		handleAutoindex(tokens, i);
	else
		throw std::runtime_error("Error: Unknown location directive: " + field);
}


/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 17:31:37 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/02/08 05:44:45 by princessj        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"

/* 공통 helper 함수 (리팩토링) */
static const Token&	directiveSyntaxCheck(const std::vector<Token>& tokens, size_t& i, const std::string& directiveName)
{
	if ((i + 1) >= tokens.size())
		throw std::runtime_error("Error: " + directiveName + " requires an argument");
	
	const Token	&tokenValue = tokens[i + 1]; // token values
	
	if (tokenValue.type != TOKEN_WORD)
		throw std::runtime_error("Error: Invalid argument for " + directiveName);
	
	i += 2;

	if (tokens[i].type != TOKEN_SEMICOLON)
		throw std::runtime_error("Error: missing ';' after " + directiveName);
	
	i++;

	return tokenValue;
}

LocationConfig::LocationConfig(const std::string &path) : _path(path), _autoindex(false) {}

LocationConfig::~LocationConfig() {}

void	LocationConfig::handleRoot(const std::vector<Token> &tokens, size_t &i)
{
	const Token	&valueToken = directiveSyntaxCheck(tokens, i, "root");
	this->_root = valueToken.value;
}

void	LocationConfig::handleAutoindex(const std::vector<Token> &tokens, size_t &i)
{
	const Token	&valueToken = directiveSyntaxCheck(tokens, i, "autoindex");
	
	if (valueToken.value == "on")
		this->_autoindex = true;
	else if (valueToken.value == "off")
		this->_autoindex = false;
	else
		throw std::runtime_error("Error: autoindex must be 'on' or 'off'");
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

void	LocationConfig::validateLocationBlock()
{
	// path 유효성 검사
	if (this->_path.empty())
		throw std::runtime_error("Error: Location path is empty");
	if (this->_path[0] != '/') // ex) location '/'upload
		throw std::runtime_error("Error: Location path must start whit '/'");
}

const std::string&	LocationConfig::getPath() const
{
	return this->_path;
}
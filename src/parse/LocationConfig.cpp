/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 17:31:37 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/02/08 07:31:23 by princessj        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"

/* 공통 helper 함수 (리팩토링) */
static const Token&	directiveSyntaxCheck(const std::vector<Token>& tokens, size_t& i, const std::string& directiveName)
{
	if ((i + 1) >= tokens.size())
		throw ConfigSyntaxException("Error: " + directiveName + " requires an argument");
	
	const Token	&tokenValue = tokens[i + 1]; // token values
	
	if (tokenValue.type != TOKEN_WORD)
		throw ConfigSyntaxException("Error: Invalid argument for " + directiveName);
	
	i += 2;

	if (tokens[i].type != TOKEN_SEMICOLON)
		throw ConfigSyntaxException("Error: missing ';' after " + directiveName);
	
	i++;

	return tokenValue;
}

LocationConfig::LocationConfig(const std::string &path) : _path(path), _root(""), _rootSet(false), _autoindex(false), _autoindexSet(false), _hasMethods(false) {}

LocationConfig::~LocationConfig() {}

void	LocationConfig::handleRoot(const std::vector<Token> &tokens, size_t &i)
{
	const Token	&valueToken = directiveSyntaxCheck(tokens, i, "root");
	this->_root = valueToken.value;
	this->_rootSet = true;
}

void	LocationConfig::handleAutoindex(const std::vector<Token> &tokens, size_t &i)
{
	const Token	&valueToken = directiveSyntaxCheck(tokens, i, "autoindex");
	
	if (valueToken.value == "on")
		this->_autoindex = true;
	else if (valueToken.value == "off")
		this->_autoindex = false;
	else
		throw ConfigSyntaxException("Error: autoindex must be 'on' or 'off'");
	
	this->_autoindexSet = true;
}


void	LocationConfig::handleMethods(const std::vector<Token>& tokens, size_t& i)
{
	this->_methods.clear(); // 초기화
	this->_hasMethods = true;

	i++; // "methods" (str) 토큰 패스. 다음 토큰 (ex) GET POST)

	while (i < tokens.size())
	{
		if (tokens[i].type == TOKEN_SEMICOLON)
		{
			i++; // ex) methods GET POST ";"
			return;
		}

		if (tokens[i].type != TOKEN_WORD) // methods 뒤에 반드시 단어가 와야함 (GET, POST, DELETE). ex) 중괄호 {} 있으면 문법 에러
			throw ConfigSyntaxException("Error: methods: invalid token");

		const std::string&	met = tokens[i].value;

		if (met != "GET" && met != "POST" && met != "DELETE") // 이 중에 없는 글자(다른 단어) 에러
			throw ConfigSyntaxException("Error: methods: unknown method " + met);

		this->_methods.push_back(met);
		i++;
	}

	throw ConfigSyntaxException("Error: methods: missing ';'");
}

void	LocationConfig::parseDirective(const std::vector<Token> &tokens, size_t &i)
{
	const std::string	&field = tokens[i].value;

	if (field == "root")
		handleRoot(tokens, i);
	else if (field == "autoindex")
		handleAutoindex(tokens, i);
	else if (field == "methods")
		handleMethods(tokens, i);
	else
		throw ConfigSemanticException("Error: Unknown location directive: " + field);
}

void	LocationConfig::validatePath() const
{
	// path 유효성 검사
	if (this->_path.empty())
		throw ConfigSemanticException("Error: Location path is empty");

	if (this->_path[0] != '/') // ex) location '/'upload
		throw ConfigSemanticException("Error: Location path must start whit '/'");
}

void	LocationConfig::validateLocationBlock()
{
	validatePath();
}

/* getters */
const std::string&	LocationConfig::getPath(void) const { return this->_path; }

bool	LocationConfig::hasRoot(void) const { return this->_rootSet; }

const std::string&	LocationConfig::getRoot(void) const { return this->_root; }

bool	LocationConfig::hasAutoindex(void) const { return this->_autoindexSet; }

bool	LocationConfig::getAutoindex(void) const { return this->_autoindex; }

bool	LocationConfig::hasMethods(void) const { return this->_hasMethods; }

const std::vector<std::string>&	LocationConfig::getMethods(void) const { return this->_methods; }
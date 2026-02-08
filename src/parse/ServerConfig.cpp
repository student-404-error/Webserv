/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 17:31:47 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/02/08 07:50:36 by princessj        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
#include "LocationConfig.hpp" // 헤더에 "LocationConfig라는 타입이 있다"만 알고 내부 구조 모름 (전방 선언)
#include <cstdlib>
#include <cctype>

/* TODO) 임시로 넣은 기본 경로 (semantic validation 목적) / 팀 협의 필요 */
static const std::string	DEFAULT_SERVER_ROOT = "./www";
static const std::string	DEFAULT_ERROR_PAGE = "/errors/404.html";

/* 공통 helper func */
static bool	isNumber(const std::string &s)
{
	if (s.empty())
		return false;
	
	for (size_t i = 0; i < s.size(); i++)
	{
		if (!std::isdigit(s[i]))
			return false;
	}

	return true;
}

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

static void	parseListenValue(const std::string& value, std::string& ip, int& port)
{
	size_t colon = value.find(':');

	if (colon == std::string::npos)
	{
		ip = "0.0.0.0";
		if (!isNumber(value))
			throw ConfigSyntaxException("Error: listen: invalid port");
		port = std::atoi(value.c_str());
	}
	else
	{
		ip = value.substr(0, colon);
		std::string portStr = value.substr(colon + 1);

		if (ip.empty() || portStr.empty() || !isNumber(portStr))
			throw ConfigSyntaxException("Error: listen: invalid ip:port");

		port = std::atoi(portStr.c_str());
	}

	if (port <= 0 || port > 65535)
		throw ConfigSemanticException("Error: listen: port out of range");
}

ServerConfig::ServerConfig() : _root(""), _errorPage(""), _hasMethods(false) {}

ServerConfig::~ServerConfig() {}

void	ServerConfig::handleListen(const std::vector<Token>& tokens, size_t& i)
{
	const Token& value = directiveSyntaxCheck(tokens, i, "listen");

	std::string ip;
	int port;

	parseListenValue(value.value, ip, port);
	checkDuplicateListen(ip, port);

	ListenAddress	addr;
	addr.ip = ip;
	addr.port = port;

	this->_listen.push_back(addr);
}

void	ServerConfig::handleRoot(const std::vector<Token> &tokens, size_t &i)
{
	const Token	&pathToken = directiveSyntaxCheck(tokens, i, "root");
	this->_root = pathToken.value;
}

void	ServerConfig::handleErrorPage(const std::vector<Token> &tokens, size_t &i)
{
	const Token	&pathToken = directiveSyntaxCheck(tokens, i, "error_page");
	this->_errorPage = pathToken.value;
}

void	ServerConfig::handleMethods(const std::vector<Token>& tokens, size_t& i)
{
	this->_methods.clear();
	this->_hasMethods = true;

	i++;

	while (i < tokens.size())
	{
		if (tokens[i].type == TOKEN_SEMICOLON)
		{
			i++;
			return;
		}

		if (tokens[i].type != TOKEN_WORD)
			throw ConfigSyntaxException("Error: methods: invalid token");

		const std::string& met = tokens[i].value;

		if (m != "GET" && met != "POST" && met != "DELETE")
			throw ConfigSyntaxException("Error: methods: unknown method " + met);

		_methods.push_back(met);
		i++;
	}

	throw ConfigSyntaxException("Error: methods: missing ';'");
}

void	ServerConfig::parseDirective(const std::vector<Token> &tokens, size_t &i)
{
	const std::string	&field = tokens[i].value;

	if (field == "listen")
		handleListen(tokens, i);
	else if (field == "root")
		handleRoot(tokens, i);
	else if (field == "error_page")
		handleErrorPage(tokens, i);
	else if (field == "methods")
		handleMethods(tokens, i);
	else
		throw ConfigSyntaxException("Error: unknown server directive: " + field);
}

/* 파싱 끝난 location 블록을 server 내부에 넣기 */
void	ServerConfig::addLocation(const LocationConfig &location)
{
	this->_locations.push_back(location);
}

void	ServerConfig::checkDuplicateListen(const std::string& ip, int port) const
{
	for (size_t i = 0; i < _listen.size(); i++)
	{
		if (this->_listen[i].ip == ip && this->_listen[i].port == port)
			throw ConfigSemanticException("Error: duplicate listen directive" );
	}
}

void	ServerConfig::validateListenDirective() const
{
	if (this->_listenPorts.empty())
		throw ConfigSemanticException("Error: Server block must contain at least 1 listen directive");
}

void	ServerConfig::applyDefaultRoot()
{
	if (this->_root.empty())
		this->_root = DEFAULT_SERVER_ROOT;
}

void	ServerConfig::applyDefaultErrorPage()
{
	if (this->_errorPage.empty())
		this->_errorPage = DEFAULT_ERROR_PAGE;
}

void	ServerConfig::duplicateLocationPathCheck() const
{
	for (size_t i = 0; i < this->_locations.size(); ++i)
	{
		for (size_t j = (i + 1); j < this->_locations.size(); ++j)
		{
			if (this->_locations[i].getPath() == this->_locations[j].getPath())
				throw ConfigSemanticException("Error: Duplicate location path: " + this->_locations[i].getPath() + "\n" + this->_locations[j].getPath());
		}
	}	
}

void	ServerConfig::validateServerBlock()
{
	validateListenDirective(); // 1) listen 필수 검사
	applyDefaultRoot(); // 2) root 기본값
	applyDefaultErrorPage(); // 3) error_page 기본값
	duplicateLocationPathCheck(); // 4) location path 중복 검사
}

/* getter */
const std::string&	ServerConfig::getRoot(void) const { return _root; }

const std::string&	ServerConfig::getErrorPage(void) const { return _errorPage; }

bool	ServerConfig::hasMethods(void) const { return _hasMethods; }

const std::vector<std::string>&	ServerConfig::getMethods(void) const { return _methods; }
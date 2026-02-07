/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 17:31:47 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/01/28 22:14:54 by princessj        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
#include "LocationConfig.hpp" // 헤더에 "LocationConfig라는 타입이 있다"만 알고 내부 구조 모름 (전방 선언)
#include <cstdlib>
#include <cctype>

/* TODO) 임시로 넣은 기본 경로 (semantic validation 목적) / 팀 협의 필요 */
static const std::string	DEFAULT_SERVER_ROOT = "./www";
static const std::string	DEFAULT_ERROR_PAGE = "/errors/404.html";

ServerConfig::ServerConfig() {}

ServerConfig::~ServerConfig() {}

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

void	ServerConfig::handleListen(const std::vector<Token> &tokens, size_t &i)
{
	if ((i + 1) >= tokens.size())
		throw std::runtime_error("Error: Listen requires a port");
	
	const Token	&portToken = tokens[i + 1];

	if (portToken.type != TOKEN_WORD || !isNumber(portToken.value))
		throw std::runtime_error("Error: Invalid listen port");
	
	int	port = std::atoi(portToken.value.c_str());

	if (port <= 0 || port > 65535)
		throw std::runtime_error("Error: Listen port out of range");
	
	for (size_t j = 0; j < this->_listenPorts.size(); j++)
	{
		if (this->_listenPorts[j] == port)
			throw std::runtime_error("Error: Duplicate listen port");
	}

	this->_listenPorts.push_back(port);

	i += 2; // listen + port
	if (tokens[i].type != TOKEN_SEMICOLON)
		throw std::runtime_error("Error: missing ';' after listen directive");
	
	i++; // ';'
}

void	ServerConfig::handleRoot(const std::vector<Token> &tokens, size_t &i)
{
	if ((i + 1) >= tokens.size())
		throw std::runtime_error("Error: root requires a path");
	
	const Token	&pathToken = tokens[i + 1];

	if (pathToken.type != TOKEN_WORD)
		throw std::runtime_error("Error: Invalid root path");
	
	this->_root = pathToken.value;

	i += 2; // root + path
	if (tokens[i].type != TOKEN_SEMICOLON)
		throw std::runtime_error("Error: missing ';' after root directive");
	
	i++;
}

void	ServerConfig::handleErrorPage(const std::vector<Token> &tokens, size_t &i)
{
	if ((i + 1) >= tokens.size())
		throw std::runtime_error("Error: Error_page requires a path");
	
	const Token	&pathToken = tokens[i + 1];

	if (pathToken.type != TOKEN_WORD)
		throw std::runtime_error("Error: Invalid error_page path");
	
	this->_errorPage = pathToken.value;

	i += 2;
	if (tokens[i].type != TOKEN_SEMICOLON)
		throw std::runtime_error("Error: missing ';' after error_page directive");
	
	i++;
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
	else
		throw std::runtime_error("Error: unknown server directive: " + field);
}

/* 파싱 끝난 location 블록을 server 내부에 넣기 */
void	ServerConfig::addLocation(const LocationConfig &location)
{
	this->_locations.push_back(location);
}

/* TODO) 기본값 팀원 협의 필요 */
void	ServerConfig::validateServerBlock()
{
	// 1) listen 필수 검사
	if (this->_listenPorts.empty())
		throw std::runtime_error("Error: Server block must contain at least 1 listen directive");
	// 2) root 기본값
	if (this->_root.empty())
		this->_root = DEFAULT_SERVER_ROOT;
	// 3) error_page 기본값
	if (this->_errorPage.empty()) 
		this->_errorPage = DEFAULT_ERROR_PAGE;
	// 4) location path 중복 검사
	for (size_t i = 0; i < this->_locations.size(); ++i)
	{
		for (size_t j = (i + 1); j < this->_locations.size(); ++j)
		{
			if (this->_locations[i].getPath() == this->_locations[j].getPath())
				throw std::runtime_error("Error: Duplicate location path: " + this->_locations[i].getPath() + "\n" + this->_locations[j].getPath());
		}
	}
}


/* Getters */
const std::vector<int>& ServerConfig::getListenPorts() const
{
	return this->_listenPorts;
}

const std::string& ServerConfig::getRoot() const
{
	return this->_root;
}

const std::string& ServerConfig::getErrorPage() const
{
	return this->_errorPage;
}

const std::vector<LocationConfig>& ServerConfig::getLocations() const
{
	return this->_locations;
}

/* For testing/manual setup */
void ServerConfig::addListenPort(int port)
{
	if (port <= 0 || port > 65535)
		throw std::runtime_error("Error: Listen port out of range");
	
	for (size_t j = 0; j < this->_listenPorts.size(); j++)
	{
		if (this->_listenPorts[j] == port)
			throw std::runtime_error("Error: Duplicate listen port");
	}
	
	this->_listenPorts.push_back(port);
}

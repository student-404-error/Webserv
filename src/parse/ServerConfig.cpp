/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 17:31:47 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/02/10 02:57:51 by princessj        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
#include "LocationConfig.hpp" // 헤더에 "LocationConfig라는 타입이 있다"만 알고 내부 구조 모름 (전방 선언)
#include <cstdlib>
#include <cctype>

/* TODO) 임시로 넣은 기본 경로 (semantic validation 목적) / 팀 협의 필요 */
static const std::string	DEFAULT_SERVER_ROOT = "./www";
static const std::string	DEFAULT_ERROR_PAGE = "/errors/404.html";
static const size_t			DEFAULT_CLIENT_MAX_BODY_SIZE = 1000000; // ex) 1MB (임시값)
static const size_t			MAX_CLIENT_BODY_SIZE = 100000000; // 100MB (임시로 넣은 값)


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

		if (met != "GET" && met != "POST" && met != "DELETE")
			throw ConfigSyntaxException("Error: methods: unknown method " + met);

		this->_methods.push_back(met);
		i++;
	}

	throw ConfigSyntaxException("Error: methods: missing ';'");
}

void	ServerConfig::handleServerName(const std::vector<Token>& tokens, size_t& i)
{
	this->_serverNames.clear();
	this->_hasServerNames = true;

	i++; // server_name 패스

	bool	hasValue = false;

	while (i < tokens.size())
	{
		if (tokens[i].type == TOKEN_SEMICOLON)
		{
			if (!hasValue)
				throw ConfigSyntaxException("Error: server_name requires at least one value");
			i++;
			return;
		}
		if (tokens[i].type != TOKEN_WORD)
			throw ConfigSyntaxException("Error: server_name: invalid token");

		hasValue = true;

		const std::string&	name = tokens[i].value;

		// server_name 중복 검사
		for (size_t j = 0; j < this->_serverNames.size(); j++)
		{
			if (this->_serverNames[j] == name)
				throw ConfigSemanticException("Error: duplicate server_name: " + name);
		}

		this->_serverNames.push_back(name);
		i++;
	}
	throw ConfigSyntaxException("Error: server_name: missing ';'");
}

void	ServerConfig::handleClientMaxBodySize(const std::vector<Token>& tokens, size_t& i)
{
	if (this->_hasClientMaxBodySize)
		throw ConfigSemanticException("Error: duplicate client_max_body_size");

	const Token&	sizeValue = directiveSyntaxCheck(tokens, i, "client_max_body_size");

	if (!isNumber(sizeValue.value))
		throw ConfigSyntaxException("Error: client_max_body_size must be a number");

	long size = std::atol(sizeValue.value.c_str()); // 문자열 -> 숫자로 저장 (int는 오버플로우 위험있음(혹은 쓰레기값), long은 음수 입력을 음수 그대로 감지 가능)

	if (size <= 0)
		throw ConfigSemanticException("Error: client_max_body_size must be > 0");
	
	if (static_cast<size_t>(size) > MAX_CLIENT_BODY_SIZE) // TODO) MAX SIZE 다시 확인하기
		throw ConfigSemanticException("Error: client_max_body_size too large");

	this->_clientMaxBodySize = static_cast<size_t>(size);
	this->_hasClientMaxBodySize = true;
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
	// TODO: listen host:port 형태 지원 시 IP 파싱 추가
	// TODO: server_name 파싱 추가 (다중 값 지원): 완료(O)
	else if (field == "server_name")
		handleServerName(tokens, i);
	else if (field == "client_max_body_size")
		handleClientMaxBodySize(tokens, i);
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
	if (this->_listen.empty())
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
	// TODO: server_name 기본값/중복 검사 (가상호스트)
}


/* Getters */
std::vector<int> ServerConfig::getListenPorts() const
{
	std::vector<int> ports;
	ports.reserve(_listen.size());
	for (size_t i = 0; i < _listen.size(); i++)
		ports.push_back(_listen[i].port);
	return ports;
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

/* validateServerBlock(): 필수인데 빠지면 서버가 동작 불가능한 것들 조건 검사
	(server_name은 필수가 아니라서(server_name은 기본값 없음) 넣지 않음: parseDirective에서 field로 있으면 넣기) */
void	ServerConfig::validateServerBlock()
{
	validateListenDirective(); // 1) listen 필수 검사
	applyDefaultRoot(); // 2) root 기본값
	applyDefaultErrorPage(); // 3) error_page 기본값
	if (!this->_hasClientMaxBodySize) // 필수는 아니지만, 런타임에서 반드시 필요
		this->_clientMaxBodySize = DEFAULT_CLIENT_MAX_BODY_SIZE;
	duplicateLocationPathCheck(); // 4) location path 중복 검사
}

/* getter */

bool	ServerConfig::hasMethods(void) const { return this->_hasMethods; }

const std::vector<std::string>&	ServerConfig::getMethods(void) const { return this->_methods; }

bool	ServerConfig::hasServerNames(void) const
{
	return this->_hasServerNames;
}

const std::vector<std::string>&	ServerConfig::getServerNames(void) const
{
	return this->_serverNames;
}

bool	ServerConfig::hasClientMaxBodySize(void) const
{
	return this->_hasClientMaxBodySize;
}

size_t	ServerConfig::getClientMaxBodySize(void) const
{
	return this->_clientMaxBodySize;
}

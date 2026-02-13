/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/25 16:03:47 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/02/08 06:18:17 by princessj        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "LocationConfig.hpp"

/* TODO) 
	1. 예외처리 따로 만들어주기
	2. validateServerBlock() / validateLocationBlock() 리팩토링 
	3. 지시문 syntax 검증 -> semantic parser에 넣기*/

Config::Config(const std::string &filePath)
{
	this->_content = openConfigFile(filePath);
	ConfigTokenizer	tokenizer;
	this->_tokens = tokenizer.tokenize(this->_content);
}

Config::~Config() {}

/*	openConfigFile()은 파일 열기(열수 있는지 검사), 파일 내부의 버퍼를 내보내는 함수로만 진행 (내부 검사 x)
	중첩구조 파싱을 더 편하게 하기 위해 파일 내부에 문자열 버퍼를 다 꺼내서 문자배열(일자형태)로 리턴이 목표 */
std::string	Config::openConfigFile(const std::string &filePath)
{
	std::ifstream	configFile(filePath.c_str());

	if (!configFile.is_open())
		throw ConfigFileException("Error: Config file open failed: " + filePath); // main에서 try,catch문으로 예외처리 예정
	
	std::stringstream	buffer;
	buffer << configFile.rdbuf(); // stream의 내부 버퍼를 꺼내는 함수: configFile 버퍼 전체 복사후 buffer에 넣기
	configFile.close(); // ifstream은 scope 종료시 자동 close (but 명시적으로 닫는 습관 만들기)
	
	return buffer.str();
}

/* i: directive의 첫 token_word를 가리켜야 함, ';'까지 문법 검증 */
/*void	Config::validateDirective(size_t &i)
{
	while (i < this->_tokens.size())
	{
		if (this->_tokens[i].type == TOKEN_SEMICOLON)
		{
			i++;
			return ; // 정상종료 : 세미콜론까지 설정 한 줄 (ex: isten 8080;)
		}
		if (this->_tokens[i].type == TOKEN_RBRACE)
			throw std::runtime_error("Error: missing ';' before '}'");
		i++;
	}
	// EOF (세미콜론 없음)
	throw std::runtime_error("Error: missing ';' at end of directive");
}*/

void	Config::configParse()
{
	LocationConfig	*currentLocation = 0; // location은 있을수도 있고, 없을 수도 있음(생성자 만들지 않음, 그래서 pointer 변수)

	try
	{
		ParseState	state = STATE_GLOBAL;
		size_t	i = 0;
		ServerConfig	currentServer; // 현재 server block
		bool	serverOpened = false;
		bool	locationOpened = false;

		while (i < this->_tokens.size())
		{
			const Token	&token = this->_tokens[i];
	
			// global state
			if (state == STATE_GLOBAL)
			{
				/* TODO) 서버 블록은 모든 지시문 앞에 '{'를 붙여야 한다. 밑에 에러 검사가 애매하게 잡혀서 추후 리팩토링 예정: validateServerBlock() / validateLocationBlock() */
				if (token.type == TOKEN_WORD && token.value == "server")
				{
					currentServer = ServerConfig(); // 새 서버 시작(생성)
					serverOpened = false;
					state = STATE_SERVER;
					i++;
					continue ;
				}
				else if (token.type == TOKEN_EOF)
				{
					if (serverOpened)
						throw std::runtime_error("Error: unclosed server block at end of file");
				
					if (locationOpened)
						throw std::runtime_error("Error: unclosed location block at end of file");
				
					break ;
				}
				else
					throw std::runtime_error("Error: unexpected '{' outside server block");
			}
			// server state
			else if (state == STATE_SERVER)
			{
				if (token.type == TOKEN_LBRACE) // {
				{
					if (serverOpened)
						throw std::runtime_error("Error: duplicate '{' in server block");
				
					serverOpened = true;
					i++;
					continue ;
				}
				else if (token.type == TOKEN_RBRACE) // }
				{
					if (!serverOpened)
						throw std::runtime_error("Error: '}' without matching '{' in server block");
				
					if (locationOpened)
						throw std::runtime_error("Error: unclosed location block before server end");
					
					// server semantic 유효성 검사: 예외가 아니면 다음에 server 저장으로 간다
					currentServer.validateServerBlock();
					
					this->_servers.push_back(currentServer); // server 저장
					serverOpened = false;
					state = STATE_GLOBAL; // 인스턴스 서버1개 끝 (중첩구조)
					i++;
					continue ;
				}
				else if (token.type == TOKEN_WORD && token.value == "location")
				{
					if (!serverOpened)
						throw std::runtime_error("Error: location outside server block"); // '{' 없는 server 통과 가능성 있음
			
					if ((i + 1) >= this->_tokens.size() || this->_tokens[i + 1].type != TOKEN_WORD) // 다음 token이 없거나 타입이 다른지 미리 syntax 검사
						throw std::runtime_error("Error: location requires a path");

					std::string	path = this->_tokens[i + 1].value; // 다음 토큰이 path여야함: ex) /upload
					currentLocation = new LocationConfig(path); // path를 가진 location 객체 생성
					// locationOpened == true  → '{'를 이미 만난 상태
					// locationOpened == false → 아직 '{'를 만나지 않은 상태
					locationOpened = false;
					i += 2; // location + path
					state = STATE_LOCATION;
					continue ;
				}
				else if (token.type == TOKEN_WORD)
				{
					if (!serverOpened)
						throw std::runtime_error("Error: directive before '{' in server block");
					currentServer.parseDirective(this->_tokens, i);
					continue ;
				}
				else
					throw std::runtime_error("Error: Invalid token in SERVER scope");
			}
			// location
			else if (state == STATE_LOCATION)
			{
				if (!currentLocation) // 안전 검사
					throw std::runtime_error("Error: Location state without location object");

				if (token.type == TOKEN_LBRACE)
				{
					if (locationOpened)
						throw std::runtime_error("Error: duplicate '{' in location block");
				
					locationOpened = true;
					i++;
					continue ;
				}
				else if (token.type == TOKEN_RBRACE)
				{
					if (!locationOpened)
						throw std::runtime_error("Error: '}' without matching '{' in location block");
			
					currentLocation->validateLocationBlock(); // location semantic 유효성 검사
					
					currentServer.addLocation(*currentLocation);
					delete currentLocation;
					currentLocation = 0; // dangling pointer 방지(초기화)
	
					locationOpened = false;
					state = STATE_SERVER; // location 위에 server
					i++;
					continue ;
				}
				else if (token.type == TOKEN_WORD)
				{
					if (!locationOpened)
						throw std::runtime_error("Error: directive before '{' in location block");
	
					currentLocation->parseDirective(this->_tokens, i);
					continue ;
				}
				else
					throw std::runtime_error("Error: Invalid token in LOCATION scope");
			}
		}
	}
	catch (...) // 어떤 타입의 예외든 다 받겠다, cleanup용(자원 정리)
	{
		if (currentLocation)
		{
			delete currentLocation;
			currentLocation = 0;
		}
		throw ;
	}
}

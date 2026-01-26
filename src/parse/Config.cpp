/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/25 16:03:47 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/01/27 00:25:43 by princessj        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

/* TODO) 예외처리 따로 만들어주기 */

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
		throw std::runtime_error("Error: Config file open failed: " + filePath); // main에서 try,catch문으로 예외처리 예정
	
	std::stringstream	buffer;
	buffer << configFile.rdbuf(); // stream의 내부 버퍼를 꺼내는 함수: configFile 버퍼 전체 복사후 buffer에 넣기
	configFile.close(); // ifstream은 scope 종료시 자동 close (but 명시적으로 닫는 습관 만들기)
	
	return buffer.str();
}

/* i: directive의 첫 token_word를 가리켜야 함, ';'까지 문법 검증 */
void	Config::validateDirective(size_t &i)
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
}

void	Config::configParse()
{
	ParseState	state = STATE_GLOBAL;
	size_t	i = 0 ;

	while (i < this->_tokens.size())
	{
		const Token	&token = this->_tokens[i];
	
		// global state
		if (state == STATE_GLOBAL)
		{
			if (token.type == TOKEN_WORD && token.value == "server")
			{
				state = STATE_SERVER;
				i++;
				continue ;
			}
			else if (token.type == TOKEN_EOF)
			{
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
				i++;
				continue ;
			}
			else if (token.type == TOKEN_RBRACE) // }
			{
				state = STATE_GLOBAL; // 인스턴스 서버1개 끝 (중첩구조)
				i++;
				continue ;
			}
			else if (token.type == TOKEN_WORD && token.value == "location")
			{
				state = STATE_LOCATION;
				i++;
				continue ;
			}
			else if (token.type == TOKEN_WORD)
			{
				validateDirective(i);
				continue ;
			}
			else
				throw std::runtime_error("Error: Invalid token in SERVER scope");
		}
		// location
		else if (state == STATE_LOCATION)
		{
			if (token.type == TOKEN_LBRACE)
			{
				i++;
				continue ;
			}
			else if (token.type == TOKEN_RBRACE)
			{
				state = STATE_SERVER; // location 위에 server
				i++;
				continue ;
			}
			else if (token.type == TOKEN_WORD)
			{
				validateDirective(i);
				continue ;
			}
			else
				throw std::runtime_error("Error: Invalid token in LOCATION scope");
		}
	}
}

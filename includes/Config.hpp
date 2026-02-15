/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/25 16:04:02 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/02/10 17:39:54 by princessj        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

/* TODO) */
/* 프로그램 시작할때 config 파일 없으면 (우리)서버가 기본으로 내장하고 있는 config 파일 불러서 열어주기 */
#define DEFAULT_CONFIG_PATH "./config/default.conf" // 임시로 넣은 경로

#include "Token.hpp"
#include "ConfigTokenizer.hpp"
#include "ServerConfig.hpp"
#include "ConfigException.hpp"
#include <fstream>
#include <sstream> // std::stringstream
#include <stdexcept>
#include <iostream>
#include <vector>

enum	ParseState
{
	STATE_GLOBAL,
	STATE_SERVER,
	STATE_LOCATION
};

/* Config 파일 전체 소유, parse state 구조 검증까지 담당 */
class	Config
{
	public:
		Config(const std::string &filePath);
		~Config(void);

		void	configParse(void);

		/* 외부에서 _servers 접근 위한 함수 (나중에 Server쪽에서 사용) */
		const std::vector<ServerConfig>&	getServers(void) const;

	private: // 논리적 + 가독성을 위해 private 2개로 분리 (C++에서 private 여러개 작성 가능)
		struct	ParserContext
		{
			ParseState		state;
			size_t			i;

			ServerConfig	currentServer;
			LocationConfig*	currentLocation;

			bool	serverOpened;
			bool	locationOpened;
			bool	expectingServerBrace; // server 이후 '{' 반드시 기다려야 하는 상태면 true, '{' 를 만나면 false (기다리지 않아도 됨)
			bool	expectingLocationBrace; // location 이후 '{' 반드시 기다려야 하는 상태면 true, '{' 를 만나면 false (기다리지 않아도 됨)

			// 생성자
			ParserContext() : state(STATE_GLOBAL), i(0), currentLocation(0), serverOpened(false),
				locationOpened(false), expectingServerBrace(false), expectingLocationBrace(false) {}
		};

		/* State handlers */
		void	handleGlobalState(ParserContext& ctx);
		void	handleServerState(ParserContext& ctx);
		void	handleLocationState(ParserContext& ctx);

		/* utils */
		std::string	openConfigFile(const std::string& filePath); // 내부 준비 작업이므로 private
		
	private:
		std::string					_content;
		std::vector<Token>			_tokens;
		std::vector<ServerConfig>	_servers;
};

#endif

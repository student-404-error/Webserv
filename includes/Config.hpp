/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/25 16:04:02 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/01/27 00:20:07 by princessj        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

/* 협의 CHECK) */
/* 프로그램 시작할때 config 파일 없으면 (우리)서버가 기본으로 내장하고 있는 config 파일 불러서 열어주기 */
#define DEFAULT_CONFIG_PATH "./config/default.conf" // 임시로 넣은 경로

#include "Token.hpp"
#include "ConfigTokenizer.hpp"
#include <fstream>
#include <sstream> // std::stringstream
#include <stdexcept>
#include <iostream>

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
		void	validateDirective(size_t &i);
	
	private:
		std::string			_content;
		std::vector<Token>	_tokens;
	
		std::string	openConfigFile(const std::string &filePath); // 내부 준비 작업이므로 private
};

#endif

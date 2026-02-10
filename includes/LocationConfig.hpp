/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 17:31:24 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/02/10 03:05:26 by princessj        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include "Token.hpp"
#include "ConfigException.hpp"
#include <vector>
#include <stdexcept>

class	LocationConfig
{
	public:
		LocationConfig(const std::string &path); // path(block identifier): location "/upload"
		~LocationConfig(void);
	
		void							parseDirective(const std::vector<Token> &tokens, size_t &i);
		void							validateLocationBlock(void); // server block 유효성 검사 함수와 동일

		/* getter */
		const std::string&				getPath(void) const;
		/* getters for runtime : 다른 파트와 이어주는 인터페이스 역할 (getter로 사용해서 보안) */
		bool							hasRoot(void) const;
		const std::string&				getRoot(void) const;
		bool							hasAutoindex(void) const;
		bool							getAutoindex(void) const;
		bool							hasMethods(void) const;
		const std::vector<std::string>&	getMethods(void) const;
		
		bool							hasIndex(void) const;
		const std::vector<std::string>&	getIndex(void) const;

	
	private:
		/* 지시문 handlers funcs */
		void	handleRoot(const std::vector<Token> &tokens, size_t &i);
		void	handleAutoindex(const std::vector<Token> &tokens, size_t &i);
		void	handleMethods(const std::vector<Token> &tokens, size_t &i);
		void	handleIndex(const std::vector<Token>& tokens, size_t& i);
	
		void	validatePath(void) const;
	
		std::string					_path;
		/* override 대상 (NOT_SET 필요) */
		std::string					_root;
		bool						_rootSet; // false = (NOT_SET): 초기화 값으로 false(NOT_SET)
		bool						_autoindex;
		bool						_autoindexSet;
		/* methods */
		std::vector<std::string>	_methods;
		bool						_hasMethods;
		
		/* location 필드 그외 */
		std::vector<std::string>	_index;
		bool						_hasIndex;
};

#endif
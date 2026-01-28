/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 17:31:24 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/01/28 22:05:43 by princessj        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include "Token.hpp"
#include <vector>
#include <stdexcept>

class	LocationConfig
{
	public:
		LocationConfig(const std::string &path); // path(block identifier): location "/upload"
		~LocationConfig(void);
	
		void	parseDirective(const std::vector<Token> &tokens, size_t &i);
		void	validateLocationBlock(void); // server block 유효성 검사 함수와 동일

		/* getter */
		const std::string&	getPath(void) const;
	
	private:
		/* 지시문 handlers funcs */
		void	handleRoot(const std::vector<Token> &tokens, size_t &i);
		void	handleAutoindex(const std::vector<Token> &tokens, size_t &i);
		/* 기본 skeleton (대표 필드): 추후 추가 예정(TODO) 회의) */
		std::string					_path;
		std::string					_root;
		bool						_autoindex;
}

#endif
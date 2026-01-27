/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 17:31:24 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/01/27 18:19:15 by princessj        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include "Token.hpp"
#include <vector>

class	LocationConfig
{
	public:
		LocationConfig(const std::string &path); // path(block identifier): location "/upload"
		~LocationConfig(void);
	
		void	parseDirective(const std::vector<Token> &tokens, size_t &i);
	
	private:
		/* 기본 skeleton (대표 필드): 추후 추가 예정 */
		std::string					_path;
		std::string					_root;
		bool						_autoindex;
		std::vector<std::string>	_allowedMethods;
}

#endif
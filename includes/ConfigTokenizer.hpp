/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigTokenizer.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 22:36:14 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/01/26 22:42:10 by princessj        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGTOKENIZER_HPP
#define CONFIGTOKENIZER_HPP

#include "Token.hpp"
#include <vector>
#include <cctype> // std::isspace

/* tokens은 config 소유이므로 여기에서는 private 멤버변수로 만들지 않음 */
class	ConfigTokenizer
{
	public:
		std::vector<Token>	tokenize(const std::string &content);
};

#endif
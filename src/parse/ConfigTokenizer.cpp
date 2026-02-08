/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigTokenizer.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 22:35:52 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/02/08 14:44:34 by jihyeki2         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigTokenizer.hpp"

std::vector<Token>	ConfigTokenizer::tokenize(const std::string &content)
{
	std::vector<Token>	tokens;
	size_t	i = 0;

	while (i < content.size())
	{
		// 1) 공백 무시
		if (std::isspace(content[i]))
		{
			i++;
			continue ;
		}
		// 2) 기호 토큰
		else if (content[i] == '{')
		{
			tokens.push_back(Token()); // tokens은 Token 타입 vector이기 때문에 Token을 넣어줘야함
			tokens.back().type = TOKEN_LBRACE; // 마지막에 넣은 토큰의 타입을 명시 (윗줄 토큰)
			tokens.back().value = "{";
			i++;
		}
		else if (content[i] == '}')
		{
			tokens.push_back(Token());
			tokens.back().type = TOKEN_RBRACE;
			tokens.back().value = "}";
			i++;
		}
		else if (content[i] == ';')
		{
			tokens.push_back(Token());
			tokens.back().type = TOKEN_SEMICOLON;
			tokens.back().value = ";";
			i++;
		}
		// 3) WORD 토큰
		else
		{
			size_t	start = i;
			
			while (i < content.size() && !std::isspace(content[i])
				&& content[i] != '{' && content[i] != '}'
				&& content[i] != ';')
				i++;
			
			tokens.push_back(Token());
			tokens.back().type = TOKEN_WORD;
			tokens.back().value = content.substr(start, (i - start));
		}
	}
	// 4) EOF
	tokens.push_back(Token());
	tokens.back().type = TOKEN_EOF;
	tokens.back().value = "";

	return tokens;
}

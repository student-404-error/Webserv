/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Token.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 22:11:46 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/02/10 18:51:30 by princessj        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

/* HTTP, config 둘다 공통으로 사용하는 token 구조체: 추후에 HTTP 토큰 타입 넣을 예정 */
enum TokenType
{
	TOKEN_WORD, // server, location, listen, root, /var/www, 8080 ...
	TOKEN_LBRACE, // {
	TOKEN_RBRACE, // }
	TOKEN_SEMICOLON, // ;
	TOKEN_EOF
};

/* C++에서는 struct = class와 동일 but 접근 제어만 기본값이 public: 생성자, 멤버 함수 있음 */
struct Token
{
	std::string	value; // 실제 텍스트
	TokenType	type;

	// 생성자 초기화
	Token() : value(""), type(TOKEN_EOF) {}
};

#endif
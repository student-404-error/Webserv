/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestValidator.hpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 22:32:56 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/02/17 02:24:16 by princessj        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUESTVALIDATOR_HPP
#define HTTPREQUESTVALIDATOR_HPP

#include "HttpRequest.hpp"
#include "HttpParseResult.hpp"
#include <sstream>
#include <cctype>

/* 이 클래스는 멤버 변수 없음, 상태 없음, 단순 검증 도구. 그래서 객체를 만들 필요가 없어서 static */
class	HttpRequestValidator
{
	public:
		static HttpParseResult	validate(const HttpRequest& request);
	
	private:
		// 내부 구현 세부사항이라 private
    	static HttpParseResult	validateContentLength(const std::map<std::string, std::string>& headers);
		static HttpParseResult	validateTransferEncodingConflict(const std::map<std::string, std::string>& headers);
};

#endif

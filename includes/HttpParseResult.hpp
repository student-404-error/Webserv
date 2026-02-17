/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParseResult.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 19:44:02 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/02/17 02:28:58 by princessj        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPARSERESULT_HPP
#define HTTPPARSERESULT_HPP

#include <cstddef>

class	HttpParseResult
{
	public:
		/* 파서 상태 */
		enum Status
		{
			PARSE_COMPLETE,
			PARSE_NEED_MORE, // 데이터가 아직 덜 들어옴
			PARSE_ERROR // 문법적으로 잘못된 요청
		};
	
		HttpParseResult(Status s, int code, size_t consumed) : _status(s), _httpStatusCode(code), _consumedLength(consumed) {}
		/* getter */
		Status	getStatus() const { return _status; }
    	int		getHttpStatusCode() const { return _httpStatusCode; }
    	size_t	getConsumedLength() const { return _consumedLength; }
	
	private:
		Status	_status; // 파서의 내부 상태
		int		_httpStatusCode; // 400, 413, 501, 505 (파서가 ERROR일때만 사용)
		size_t	_consumedLength; // 실제 계산은 HttpRequest. HttpRequestValidator가 가져와서 여기에 담음
};

#endif
